#include "MemMgr.h"

#include <hj/ai/vector_index.hpp>
#include <hj/algo/uuid.hpp>

#include <libqt/encoding/json.h>

#include "GrpcClient.h"
#include "Account.h"
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

    QFileInfo indexFile(QString::fromStdString(indexFilePath));
    auto      indexDir = indexFile.absoluteDir();
    if(!indexDir.exists())
    {
        if(!indexDir.mkpath("."))
        {
            qWarning() << "Failed to create index directory:"
                       << indexDir.absolutePath();
            return false;
        }
    }

    QFileInfo metaFile(QString::fromStdString(metaFilePath));
    QDir      metaDir = metaFile.absoluteDir();
    if(!metaDir.exists())
    {
        if(!metaDir.mkpath("."))
        {
            qWarning() << "Failed to create meta directory:"
                       << metaDir.absolutePath();
            return false;
        }
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

int64_t MemMgr::asyncRetrieve(const QString &text,
                              const int      topK,
                              const QString &memoryId)
{
    // Implementation goes here
    // 1. embedding question
    // 2. retrieve from memoryId
    // 3. index results to meta file
    // 4. return results
    auto conf   = Config::instance()->getMemoryConfigById(memoryId);
    auto taskId = std::abs(static_cast<int64_t>(hj::uuid::gen_u64()));
    _addEmbeddingTask(taskId, 1, conf.id);
    _addRetrieveTask(taskId, text, topK, conf.id);
    GrpcClient::instance()->Embedding(taskId,
                                      Account::instance()->id(),
                                      Account::instance()->auth(),
                                      0,
                                      text.toUtf8(),
                                      0,
                                      text.size() - 1,
                                      conf);
    qDebug() << "MemMgr::asyncRetrieve with taskId:" << taskId
             << ", topK:" << topK;
    return taskId;
}

int64_t MemMgr::asyncEmbedding(const QStringList          &files,
                               const Config::MemoryConfig &conf)
{
    int64_t     taskId = std::abs(static_cast<int64_t>(hj::uuid::gen_u64()));
    FileChunker chunker;
    auto        chunkCount = chunker.chunkFiles(
        files,
        conf.chunkSize,
        [this, taskId, conf](FileChunker::Chunk &chunk) -> qint64 {
            if(chunk.id == 0 && chunk.startPos == 0)
            {
                // first chunk, set param (no data)
                qDebug() << "file: " << chunk.filePathName
                         << " first chunk, add Embedding task";
                _addEmbeddingTask(taskId, -1, conf.id);
                GrpcClient::instance()->Embedding(taskId,
                                                  Account::instance()->id(),
                                                  Account::instance()->auth(),
                                                  chunk.id,
                                                  chunk.data,
                                                  chunk.startPos,
                                                  chunk.startPos + chunk.offset,
                                                  conf);
            }

            // calculate next chunk start position and offset
            qint64 startPos = chunk.startPos + chunk.offset - conf.overlap;
            if(startPos <= chunk.startPos)
                startPos = chunk.startPos + chunk.offset; // ensure progress

            auto    totalLength = File::fileSize(chunk.filePathName);
            qint64  endPos      = qMin(startPos + chunk.offset, totalLength);
            QString text        = QString::fromUtf8(chunk.data);
            int     textSize    = text.size();
            if(conf.respectParagraphs && textSize > 0)
            {
                qint64 newEndPos =
                    startPos + File::findParagraphBoundary(text, 0, textSize);
                if(newEndPos < endPos && newEndPos > startPos)
                    endPos = newEndPos;
            }

            if(conf.respectSentences && textSize > 0)
            {
                qint64 newEndPos =
                    startPos + File::findSentenceBoundary(text, 0, textSize);
                if(newEndPos < endPos && newEndPos > startPos)
                    endPos = newEndPos;
            }

            chunk.startPos = startPos;
            chunk.offset   = endPos - startPos;
            chunk.id       = static_cast<int64_t>(hj::uuid::gen_u64());
            qDebug() << "Generated chunk: index=" << chunk.id
                     << ", startPos=" << chunk.startPos
                     << ", offset=" << chunk.offset
                     << ", dataSize=" << chunk.data.size();

            if(chunk.data.isEmpty() || chunk.offset <= 0)
            {
                qDebug() << "Chunk data is empty or offset is non-positive, "
                            "skipping.";
                return false;
            }

            // build meta file
            auto memoryId = conf.id;
            add(memoryId.toStdString(), chunk);

            // send chunk
            qDebug() << "send chunk data with id:" << chunk.id
                     << ", startPos:" << chunk.startPos;
            GrpcClient::instance()->Embedding(taskId,
                                              Account::instance()->id(),
                                              Account::instance()->auth(),
                                              chunk.id,
                                              chunk.data,
                                              chunk.startPos,
                                              chunk.startPos + chunk.offset);
            return true;
        });

    qDebug() << "MemMgr::asyncEmbedding with taskId:" << taskId;
    _setEmbeddingChunkNum(taskId, chunkCount);
    return taskId;
}

int64_t MemMgr::asyncEmbedding(const int64_t               taskId,
                               const FileChunker::Chunk   &chunk,
                               const Config::MemoryConfig &conf)
{
    m_mu.lock();
    if(m_mapEmbeddingTasks.find(taskId) == m_mapEmbeddingTasks.end())
        _addEmbeddingTask(taskId, -1, conf.id);
    m_mu.unlock();

    GrpcClient::instance()->Embedding(taskId,
                                      Account::instance()->id(),
                                      Account::instance()->auth(),
                                      chunk.id,
                                      chunk.data,
                                      chunk.startPos,
                                      chunk.startPos + chunk.offset,
                                      conf);
    qDebug() << "MemMgr::asyncEmbedding with taskId:" << taskId;
    return taskId;
}

void MemMgr::stopEmbedding(const int64_t taskId)
{
    m_mu.lock();
    if(m_mapEmbeddingTasks.find(taskId) != m_mapEmbeddingTasks.end())
    {
        GrpcClient::instance()->EmbeddingStop(taskId,
                                              Account::instance()->id(),
                                              Account::instance()->auth());
    }
    m_mu.unlock();
}

void MemMgr::slotEmbeddingResp(const int         errorCode,
                               const int64_t     taskId,
                               const int64_t     chunkId,
                               const QByteArray &vectorIndexs)
{
    qDebug() << "slotEmbeddingResp get taskId:" << taskId
             << ", vectorIndexs.size():" << vectorIndexs.size();
    std::lock_guard<std::mutex> guard(m_mu);
    if(m_mapEmbeddingTasks.find(taskId) == m_mapEmbeddingTasks.end())
    {
        // qDebug() << "embedding task:" << taskId << " not exist!";
        return;
    }

    if(errorCode != 0)
    {
        qWarning() << "Embedding response error, code: " << errorCode;
        // stop embedding for this task
        GrpcClient::instance()->EmbeddingStop(taskId,
                                              Account::instance()->id(),
                                              Account::instance()->auth());
    }

    // skip param setting resp
    if(chunkId == 0 && vectorIndexs.isEmpty())
    {
        qDebug() << "Embedding response set param, skip.";
        return;
    }

    // add record
    _addEmbeddedChunk(taskId, chunkId, vectorIndexs);
    if(!_isEmbeddingFinished(taskId))
    {
        qDebug() << "retrieve not finished, continue.";
        return;
    }

    // handle retrieve task
    if(m_mapRetrieveTasks.find(taskId) != m_mapRetrieveTasks.end())
    {
        RetrieveTask task     = m_mapRetrieveTasks[taskId];
        auto         text     = task.text;
        auto         topK     = task.topK;
        auto         memoryId = task.memoryId;
        auto         conf   = Config::instance()->getMemoryConfigById(memoryId);
        auto         chunks = _retrieve(vectorIndexs, topK, conf);

        QVector<QJsonObject> memorys;
        for(auto chunk : chunks)
        {
            QJsonObject obj;
            convert(obj, chunk);
            memorys.append(obj);
        }

        qDebug() << "send retrieve finished state with taskId:" << taskId;
        emit signalRetrieveFinished(OK, taskId, text, topK, memoryId, memorys);
    }

    // handle embedding task
    if(m_mapEmbeddingTasks.find(taskId) != m_mapEmbeddingTasks.end())
    {
        auto memoryId = m_mapEmbeddingTasks[taskId].memoryId;
        qDebug() << "send embedding finished state with taskId:" << taskId;
        emit signalEmbeddingFinished(OK, taskId, memoryId);
    }

    // stop embedding for this task
    GrpcClient::instance()->EmbeddingStop(taskId,
                                          Account::instance()->id(),
                                          Account::instance()->auth());
}

void MemMgr::signalStopEmbeddingResp(const int errorCode, const int64_t taskId)
{
    if(errorCode != 0)
    {
        qWarning() << "Fail to stop embedding with errorCode:" << errorCode;
        return;
    }

    _removeEmbeddingTask(taskId);
    _removeRetrieveTask(taskId);
}

void MemMgr::_init()
{
    // init connections
    _initConnections();

    // Initialization memory file
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

void MemMgr::_initConnections()
{
    connect(GrpcClient::instance(),
            &GrpcClient::signalEmbeddingResp,
            this,
            &MemMgr::slotEmbeddingResp);

    connect(GrpcClient::instance(),
            &GrpcClient::signalStopEmbeddingResp,
            this,
            &MemMgr::signalStopEmbeddingResp);
}

QVector<FileChunker::Chunk> MemMgr::_retrieve(const QByteArray &embeddings,
                                              const int         topK,
                                              const Config::MemoryConfig &conf)
{
    QVector<FileChunker::Chunk> chunks;
    auto                        dimension = conf.dimension;
    std::vector<uint8_t>        buffer(embeddings.begin(), embeddings.end());
    std::vector<float>          vectors;
    if(!convert(vectors, std::move(buffer), dimension))
    {
        qDebug() << "Failed to convert embedding data to float vectors.";
        return chunks;
    }

    return _retrieve(vectors, topK, conf.id.toStdString());
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

bool MemMgr::_isEmbeddingFinished(const int64_t taskId)
{
    auto itr = m_mapEmbeddingTasks.find(taskId);
    if(itr == m_mapEmbeddingTasks.end())
        return true;

    return itr->second.totalChunkNum != -1
           && itr->second.finishedChunkIds.size() >= itr->second.totalChunkNum;
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

void MemMgr::_addEmbeddingTask(const int64_t  taskId,
                               const int      totalChunkNum,
                               const QString &memoryId)
{
    if(m_mapEmbeddingTasks.find(taskId) != m_mapEmbeddingTasks.end())
        return;

    EmbeddingTask task;
    task.id            = taskId;
    task.totalChunkNum = totalChunkNum;
    task.memoryId      = memoryId;

    m_mapEmbeddingTasks[taskId] = task;
}

void MemMgr::_addRetrieveTask(const int64_t  taskId,
                              const QString &text,
                              const int      topK,
                              const QString &memoryId)
{
    if(m_mapRetrieveTasks.find(taskId) != m_mapRetrieveTasks.end())
        return;

    RetrieveTask task;
    task.id       = taskId;
    task.text     = text;
    task.topK     = topK;
    task.memoryId = memoryId;

    m_mapRetrieveTasks[taskId] = task;
}

void MemMgr::_removeEmbeddingTask(const int64_t taskId)
{
    m_mapEmbeddingTasks.erase(taskId);
}

void MemMgr::_removeRetrieveTask(const int64_t taskId)
{
    m_mapRetrieveTasks.erase(taskId);
}

void MemMgr::_addEmbeddedChunk(const int64_t     taskId,
                               const int64_t     chunkId,
                               const QByteArray &vectorIndexs)
{
    if(m_mapEmbeddingTasks.find(taskId) == m_mapEmbeddingTasks.end())
        return;

    m_mapEmbeddingTasks[taskId].finishedChunkIds.insert(chunkId);
    emit signalEmbeddingProgress(
        taskId,
        chunkId,
        m_mapEmbeddingTasks[taskId].memoryId,
        m_mapEmbeddingTasks[taskId].totalChunkNum,
        m_mapEmbeddingTasks[taskId].finishedChunkIds.size(),
        vectorIndexs);
}

void MemMgr::_setEmbeddingChunkNum(const int64_t taskId,
                                   const int     totalChunkNum)
{
    if(m_mapEmbeddingTasks.find(taskId) == m_mapEmbeddingTasks.end())
        return;

    m_mapEmbeddingTasks[taskId].totalChunkNum = totalChunkNum;
}