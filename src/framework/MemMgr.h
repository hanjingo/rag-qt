#ifndef MEMMGR_H
#define MEMMGR_H

#include <QObject>

#include <unordered_map>
#include <string>
#include <mutex>
#include <QPointer>
#include <QVector>
#include <QJsonObject>

#include <hj/ai/vector_index.hpp>
#include <hj/sync/channel.hpp>

#include <libqt/io/file.h>
#include <libqt/io/filechunker.h>

#include "Config.h"

class MemMgr : public QObject
{
    Q_OBJECT

  public:
    struct RetrieveTask
    {
        int64_t id;
        int     topK;
        QString text;
        QString memoryId;
    };

    struct EmbeddingTask
    {
        int64_t       id;
        int           totalChunkNum = -1;
        QSet<int64_t> finishedChunkIds;
        QString       memoryId;
    };

  public:
    explicit MemMgr(QObject *parent = nullptr) { _init(); };
    ~MemMgr() {};

    static QPointer<MemMgr> instance()
    {
        static QPointer<MemMgr> inst = new MemMgr();
        return inst;
    }

    bool add(const std::string     &memoryId,
             std::vector<uint8_t> &&embedding,
             const int              dimension,
             const int64_t          chunkId);
    bool add(const std::string &memoryId, const FileChunker::Chunk &chunk);

    bool load(const std::string &memoryId,
              const std::string &indexFilePath,
              const std::string &metaFilePath);

    bool save(const std::string &memoryId,
              const std::string &indexFilePath,
              const std::string &metaFilePath);

    int64_t
    asyncRetrieve(const QString &text, const int topK, const QString &memoryId);

    int64_t asyncEmbedding(const QStringList          &files,
                           const Config::MemoryConfig &conf);

    int64_t
    asyncEmbedding(const int64_t               taskId,
                   const FileChunker::Chunk   &chunk,
                   const Config::MemoryConfig &conf = Config::MemoryConfig());

    void stopEmbedding(const int64_t taskId);

    bool convert(FileChunker::Chunk &dst, const QJsonObject &src);
    bool convert(QJsonObject &dst, const FileChunker::Chunk &src);
    bool convert(std::vector<float>    &dst,
                 std::vector<uint8_t> &&src,
                 const int              dimension);

  signals:
    void signalEmbeddingProgress(const int64_t     taskId,
                                 const int64_t     chunkId,
                                 const QString    &memoryId,
                                 const int         totalChunkNum,
                                 const int         finishedChunkNum,
                                 const QByteArray &vectorIndexs);

    void signalRetrieveFinished(const int             errorCode,
                                const int64_t         taskId,
                                const QString        &text,
                                const int             topK,
                                const QString        &memoryId,
                                QVector<QJsonObject> &chunks);

    void signalEmbeddingFinished(const int      errorCode,
                                 const int64_t  taskId,
                                 const QString &memoryId);

  private slots:
    void slotEmbeddingResp(const int         errorCode,
                           const int64_t     taskId,
                           const int64_t     chunkId,
                           const QByteArray &vectorIndexs);

    void signalStopEmbeddingResp(const int errorCode, const int64_t taskId);

  private:
    void _init();
    void _initConnections();

    QVector<FileChunker::Chunk> _retrieve(const QByteArray &embeddings,
                                          const int         topK,
                                          const Config::MemoryConfig &conf);
    QVector<FileChunker::Chunk> _retrieve(const std::vector<float> embeddings,
                                          const int                topK,
                                          const std::string       &memoryId);

    bool _isEmbeddingFinished(const int64_t taskId);

    void _addEmbeddingTask(const int64_t  taskId,
                           const int      totalChunkNum,
                           const QString &memoryId);
    void _addRetrieveTask(const int64_t  taskId,
                          const QString &text,
                          const int      topK,
                          const QString &memoryId);

    void _removeEmbeddingTask(const int64_t taskId);
    void _removeRetrieveTask(const int64_t taskId);

    void _addEmbeddedChunk(const int64_t     taskId,
                           const int64_t     chunkId,
                           const QByteArray &vectorIndexs);

    void _setEmbeddingChunkNum(const int64_t taskId, const int totalChunkNum);

  private:
    std::unordered_map<std::string, hj::vector_index<hj::vindex_idmap_t>>
                                                 m_mapIndexes;
    std::unordered_map<std::string, QJsonObject> m_mapMetas;

    std::mutex                                 m_mu;
    std::unordered_map<int64_t, RetrieveTask>  m_mapRetrieveTasks;
    std::unordered_map<int64_t, EmbeddingTask> m_mapEmbeddingTasks;
};

#endif // MEMMGR_H