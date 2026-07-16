#include "MemMgr.h"

#include <hj/ai/vector_index.hpp>

#include "GrpcClient.h"
#include "Account.h"

bool MemMgr::add(const std::string     &memoryId,
                 std::vector<uint8_t> &&embedding,
                 const int              dimension)
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

    if(m_mapIndexes.find(memoryId) == m_mapIndexes.end())
    {
        hj::vector_index<hj::vindex_flat_l2_t> mainIndex;
        mainIndex.build(dimension);
        m_mapIndexes[memoryId] = std::move(mainIndex);
    }

    auto              &mainIndex = m_mapIndexes[memoryId];
    std::vector<float> vectors;
    if(!index.get_all_vectors(vectors))
    {
        qDebug() << "Failed to get vectors from chunk index";
        return false;
    }

    int numVectors = vectors.size() / dimension;
    mainIndex.add(numVectors, vectors.data());
    qDebug() << "Added" << numVectors << "vectors to memoryId:" << memoryId
             << ", total:" << mainIndex.total();
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

    if(!File::isFileExist(QString::fromStdString(indexFilePath))
       || !File::isFileExist(QString::fromStdString(metaFilePath)))
    {
        qDebug() << "Index file or meta file does not exist: " << indexFilePath
                 << ", " << metaFilePath
                 << " skipping load for memoryId: " << memoryId;
        return false;
    }

    hj::vector_index<hj::vindex_flat_l2_t> index;
    if(!index.load(indexFilePath.c_str()))
    {
        qDebug() << "Failed to load index file: " << indexFilePath;
        return false;
    }
    m_mapIndexes[memoryId] = std::move(index);

    QJsonObject meta;
    if(!File::readJsonFile(QString::fromStdString(metaFilePath), meta))
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
        qDebug() << "Failed to save combined index file: " << indexFilePath;
        return false;
    }

    auto &meta = m_mapMetas[memoryId];
    File::writeJsonFile(QString::fromStdString(metaFilePath), meta);
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
    task.question = QString::fromStdString(question);
    task.topK     = topK;
    task.memoryId = QString::fromStdString(memoryId);
    m_retrieveCh.enqueue(task);
    _sendTask();
}

void MemMgr::SlotEmbeddingResp(const int         errorCode,
                               const int64_t     taskId,
                               const int64_t     chunkId,
                               const QByteArray &vectorIndexs)
{
    _sendTask();
    if(errorCode != 0)
    {
        qDebug() << "Embedding response error, code: " << errorCode;
        return;
    }

    RetrieveTask task;
    if(!m_retrieveCh.try_dequeue(task))
    {
        qDebug() << "No retrieve task found for embedding response.";
        return;
    }

    auto conf = Config::Instance().getMemoryConfigById(task.memoryId);
    if(conf.id.isEmpty())
    {
        qDebug() << "Memory config not found for memoryId: " << task.memoryId;
        return;
    }

    auto                 topK      = task.topK;
    auto                 memoryId  = task.memoryId.toStdString();
    auto                 dimension = conf.dimension;
    std::vector<uint8_t> buffer(vectorIndexs.begin(), vectorIndexs.end());
    std::vector<float>   vectors;
    if(!convert(vectors, std::move(buffer), dimension))
    {
        qDebug() << "Failed to convert embedding data to float vectors.";
        return;
    }
    _retrieve(vectors, topK, memoryId);
}

void MemMgr::_init()
{
    // Initialization code here
    auto confs = Config::Instance().memoryConfigs();
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

void MemMgr::_sendTask()
{
    RetrieveTask task;
    if(!m_retrieveCh.try_dequeue(task))
        return;

    auto conf = Config::Instance().getMemoryConfigById(task.memoryId);
    GrpcClient::Instance()->Embedding(0,
                                      Account::Instance()->Id(),
                                      Account::Instance()->Auth(),
                                      0,
                                      task.question.toUtf8(),
                                      0,
                                      task.question.size(),
                                      conf);
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
        if(idx < 0 || idx >= meta.size())
            continue;

        auto               obj = meta[QString::number(idx)].toObject();
        FileChunker::Chunk chunk;
        convert(chunk, obj);
        results.append(chunk);
        qDebug() << "Retrieved from memoryId: " << memoryId
                 << ", index: " << idx
                 << ", content: " << obj.value("content").toString()
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