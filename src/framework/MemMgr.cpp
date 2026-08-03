#include "MemMgr.h"

#include <hj/ai/vector_index.hpp>
#include <hj/algo/uuid.hpp>

#include <libqt/encoding/json.h>

#include "GrpcClient.h"
#include "Account.h"
#include "BusAdapter.h"
#include "Error.h"

bool MemMgr::add(const std::string     &memoryId,
                 std::vector<uint8_t> &&embedding,
                 const int              dimension,
                 const int64_t          chunkId)
{
    if(embedding.empty() || dimension <= 0)
    {
        qDebug() << "Invalid embedding data: size=" << embedding.size()
                 << ", dimension=" << dimension;
        return false;
    }

    hj::vector_index<hj::vindex_flat_l2_t> index;
    if(!index.deserialize(std::move(embedding)))
    {
        qDebug() << "Failed to deserialize embedding data";
        return false;
    }

    std::vector<float> vectors;
    if(!index.get_all_vectors(vectors))
    {
        qDebug() << "Failed to get vectors from chunk index";
        return false;
    }

    int numVectors = vectors.size() / dimension;
    if(m_mapIndexes.find(memoryId) == m_mapIndexes.end())
    {
        hj::vector_index<hj::vindex_idmap_t> mainIndex(
            new hj::vindex_idmap_t(new hj::vindex_flat_l2_t(dimension)));
        m_mapIndexes[memoryId] = std::move(mainIndex);
    }

    auto                     &mainIndex = m_mapIndexes[memoryId];
    std::vector<faiss::idx_t> ids       = {chunkId};

    if(!mainIndex.add_with_ids(numVectors, vectors.data(), ids.data()))
    {
        qDebug() << "Added vector with chunk_id:" << chunkId
                 << "to memoryId:" << memoryId
                 << ", total:" << mainIndex.total();
        return false;
    }

    qDebug() << "Added vector with chunk_id:" << chunkId
             << "to memoryId:" << memoryId << ", total:" << mainIndex.total();
    return true;
}

bool MemMgr::add(const std::string &memoryId, const FileChunker::Chunk &chunk)
{
    if(m_mapMetas.find(memoryId) == m_mapMetas.end())
    {
        qDebug() << "add chunkMemory ID not found: " << memoryId
                 << " created it";
        m_mapMetas[memoryId] = QJsonObject();
    }

    auto       &meta = m_mapMetas[memoryId];
    QJsonObject obj;
    convert(obj, chunk);
    meta.insert(QString::number(chunk.id), obj);

    return true;
}

bool MemMgr::load(const std::string &memoryId,
                  const std::string &indexFilePath,
                  const std::string &metaFilePath)
{
    if(m_mapIndexes.find(memoryId) != m_mapIndexes.end()
       || m_mapMetas.find(memoryId) != m_mapMetas.end())
    {
        qDebug() << "Memory ID already loaded: " << memoryId;
        return false;
    }

    if(!File::isExist(QString::fromStdString(indexFilePath))
       || !File::isExist(QString::fromStdString(metaFilePath)))
    {
        qDebug() << "Index file or meta file does not exist: " << indexFilePath
                 << ", " << metaFilePath
                 << " skipping load for memoryId: " << memoryId;
        return false;
    }

    hj::vector_index<hj::vindex_idmap_t> index;
    if(!index.load(indexFilePath.c_str()))
    {
        qDebug() << "Failed to load index file: " << indexFilePath;
        return false;
    }
    m_mapIndexes[memoryId] = std::move(index);

    QJsonObject meta;
    if(!JSON::readFile(meta, QString::fromStdString(metaFilePath)))
    {
        qDebug() << "Failed to load meta file: " << metaFilePath;
        return false;
    }
    m_mapMetas[memoryId] = meta;
    return true;
}

bool MemMgr::save(const std::string &memoryId,
                  const std::string &indexFilePath,
                  const std::string &metaFilePath)
{
    if(m_mapIndexes.find(memoryId) == m_mapIndexes.end()
       || m_mapMetas.find(memoryId) == m_mapMetas.end())
    {
        qDebug() << "save Memory ID not found: " << memoryId;
        return false;
    }

    auto &index = m_mapIndexes[memoryId];
    if(!index.save(indexFilePath.c_str()))
    {
        qDebug() << "Failed to save combined index file befor save: "
                 << indexFilePath;
        return false;
    }

    auto &meta = m_mapMetas[memoryId];
    JSON::writeFile(QString::fromStdString(metaFilePath), meta);
    qDebug() << "Saved index and meta files for memoryId: " << memoryId
             << ", indexFilePath: " << indexFilePath
             << ", metaFilePath: " << metaFilePath;
    return true;
}

void MemMgr::retrieve(const std::string &question,
                      const int          topK,
                      const std::string &memoryId)
{
    // Implementation goes here
    // 1. embedding question
    // 2. retrieve from memoryId
    // 3. index results to meta file
    // 4. return results

    RetrieveTask task;
    task.id       = std::abs(static_cast<int64_t>(hj::uuid::gen_u64()));
    task.question = QString::fromStdString(question);
    task.topK     = topK;
    task.memoryId = QString::fromStdString(memoryId);

    m_mu.lock();
    m_mapTasks[task.id] = task;
    m_mu.unlock();

    auto conf = Config::instance()->getMemoryConfigById(task.memoryId);
    GrpcClient::Instance()->Embedding(task.id,
                                      Account::Instance()->Id(),
                                      Account::Instance()->Auth(),
                                      1,
                                      task.question.toUtf8(),
                                      0,
                                      task.question.size() - 1,
                                      conf);
}

void MemMgr::SlotEmbeddingResp(const int         errorCode,
                               const int64_t     taskId,
                               const int64_t     chunkId,
                               const QByteArray &vectorIndexs)
{
    std::lock_guard<std::mutex> guard(m_mu);
    if(m_mapTasks.find(taskId) == m_mapTasks.end())
    {
        return;
    }

    qDebug() << "SlotEmbeddingResp get taskId:" << taskId
             << ", vectorIndexs.size():" << vectorIndexs.size();
    RetrieveTask task     = m_mapTasks[taskId];
    auto         question = task.question;
    auto         topK     = task.topK;
    auto         memoryId = task.memoryId;
    if(errorCode != 0)
    {
        qDebug() << "Embedding response error, code: " << errorCode;
        // notify bus
        emit BusAdapter::Instance()
            -> SignalRetrieveResp(errorCode, question, topK, memoryId, {});

        // remove record
        m_mapTasks.erase(taskId);

        // stop embedding for this task
        GrpcClient::Instance()->EmbeddingStop(taskId,
                                              Account::Instance()->Id(),
                                              Account::Instance()->Auth());
        return;
    }

    if(chunkId == 0)
    {
        qDebug() << "Embedding response set param, skip.";
        return;
    }

    // remove record first
    m_mapTasks.erase(taskId);
    // stop embedding for this task first
    GrpcClient::Instance()->EmbeddingStop(taskId,
                                          Account::Instance()->Id(),
                                          Account::Instance()->Auth());

    auto conf = Config::instance()->getMemoryConfigById(memoryId);
    if(conf.id.isEmpty())
    {
        qDebug() << "Memory config not found for memoryId: " << memoryId;
        return;
    }

    auto                 dimension = conf.dimension;
    std::vector<uint8_t> buffer(vectorIndexs.begin(), vectorIndexs.end());
    std::vector<float>   vectors;
    if(!convert(vectors, std::move(buffer), dimension))
    {
        qDebug() << "Failed to convert embedding data to float vectors.";
        return;
    }
    QVector<FileChunker::Chunk> chunks =
        _retrieve(vectors, topK, memoryId.toStdString());
    QVector<QJsonObject> memorys;
    for(auto chunk : chunks)
    {
        QJsonObject obj;
        convert(obj, chunk);
        memorys.append(obj);
    }
    emit BusAdapter::Instance()
        -> SignalRetrieveResp(OK, task.question, topK, memoryId, memorys);
}

void MemMgr::_init()
{
    connect(GrpcClient::Instance(),
            &GrpcClient::SignalEmbeddingResp,
            this,
            &MemMgr::SlotEmbeddingResp);

    // Initialization code here
    auto confs = Config::instance()->memoryConfigs();
    for(auto conf : confs)
    {
        auto memoryId      = conf.id.toStdString();
        auto indexFilePath = conf.indexFilePath.toStdString();
        auto metaFilePath  = conf.metaFilePath.toStdString();
        if(!load(memoryId, indexFilePath, metaFilePath))
        {
            qDebug() << "Failed to load memory: " << memoryId;
            continue;
        }
    }
}

QVector<FileChunker::Chunk>
MemMgr::_retrieve(const std::vector<float> embeddings,
                  const int                topK,
                  const std::string       &memoryId)
{
    QVector<FileChunker::Chunk> results;
    if(m_mapIndexes.count(memoryId) == 0 || m_mapMetas.count(memoryId) == 0)
    {
        qDebug() << "retrieve Memory ID not found: " << memoryId;
        return results;
    }

    auto &index = m_mapIndexes[memoryId];
    auto &meta  = m_mapMetas[memoryId];

    std::vector<float>            distances(topK);
    std::vector<hj::vindex_idx_t> indices(topK);
    index.search(1, embeddings.data(), topK, distances.data(), indices.data());

    for(int i = 0; i < topK; ++i)
    {
        auto idx = indices[i];
        qDebug() << "idx:" << idx;
        auto key = QString::number(idx);
        if(!meta.contains(key))
            continue;

        auto               obj = meta[key].toObject();
        FileChunker::Chunk chunk;
        convert(chunk, obj);
        results.append(chunk);
        qDebug() << "Retrieved from memoryId: " << memoryId
                 << ", index: " << idx
                 << ", data: " << obj.value("data").toString()
                 << ", distance: " << distances[i];
    }

    return results;
}

bool MemMgr::convert(FileChunker::Chunk &dst, const QJsonObject &src)
{
    dst.id           = src.value("chunk_id").toString().toLongLong();
    dst.data         = src.value("data").toString().toUtf8();
    dst.startPos     = src.value("start_pos").toString().toLongLong();
    dst.offset       = src.value("offset").toString().toLongLong();
    dst.filePathName = src.value("file_path_name").toString();
    dst.timestamp    = src.value("timestamp").toString();
    return true;
}

bool MemMgr::convert(QJsonObject &dst, const FileChunker::Chunk &src)
{
    dst["chunk_id"]       = QString::number(src.id);
    dst["data"]           = QString::fromUtf8(src.data);
    dst["start_pos"]      = QString::number(src.startPos);
    dst["end_pos"]        = QString::number(src.startPos + src.offset);
    dst["chunk_size"]     = QString::number(src.offset);
    dst["file_path_name"] = src.filePathName;
    dst["timestamp"]      = src.timestamp;
    return true;
}

bool MemMgr::convert(std::vector<float>    &dst,
                     std::vector<uint8_t> &&src,
                     const int              dimension)
{
    if(src.empty() || dimension <= 0)
        return false;

    hj::vector_index<hj::vindex_flat_l2_t> index;
    if(!index.deserialize(std::move(src)))
        return false;

    if(!index.get_all_vectors(dst))
        return false;

    return true;
}