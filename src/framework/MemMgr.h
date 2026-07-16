#ifndef MEMMGR_H
#define MEMMGR_H

#include <QObject>

#include <unordered_map>
#include <string>

#include <hj/ai/vector_index.hpp>
#include <hj/sync/channel.hpp>

#include "Config.h"
#include "File.h"
#include "FileChunker.h"

class MemMgr : public QObject
{
    Q_OBJECT

  public:
    struct RetrieveTask
    {
        int     topK;
        QString question;
        QString memoryId;
    };

  public:
    explicit MemMgr(QObject *parent = nullptr)
        : m_retrieveCh{Config::Instance().getRetrieveTaskQueueSize()}
    {
        _init();
    };
    ~MemMgr() {};

    static MemMgr *Instance()
    {
        static MemMgr instance;
        return &instance;
    }

    bool add(const std::string     &memoryId,
             std::vector<uint8_t> &&embedding,
             const int              dimension);
    bool add(const std::string &memoryId, const FileChunker::Chunk &chunk);

    bool load(const std::string &memoryId,
              const std::string &indexFilePath,
              const std::string &metaFilePath);

    bool save(const std::string &memoryId,
              const std::string &indexFilePath,
              const std::string &metaFilePath);

    void retrieve(const std::string &question,
                  const int          topK,
                  const std::string &memoryId);

    bool convert(FileChunker::Chunk &dst, const QJsonObject &src);
    bool convert(QJsonObject &dst, const FileChunker::Chunk &src);
    bool convert(std::vector<float>    &dst,
                 std::vector<uint8_t> &&src,
                 const int              dimension);

  signals:
    void SignalRetrieveResp();

  public slots:
    void SlotEmbeddingResp(const int         errorCode,
                           const int64_t     taskId,
                           const int64_t     chunkId,
                           const QByteArray &vectorIndexs);

  private:
    void                        _init();
    void                        _sendTask();
    QVector<FileChunker::Chunk> _retrieve(const std::vector<float> embeddings,
                                          const int                topK,
                                          const std::string       &memoryId);

  private:
    std::unordered_map<std::string, hj::vector_index<hj::vindex_flat_l2_t>>
                                                 m_mapIndexes;
    std::unordered_map<std::string, QJsonObject> m_mapMetas;

    hj::channel<RetrieveTask> m_retrieveCh;
};

#endif // MEMMGR_H