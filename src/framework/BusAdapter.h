#ifndef BUSADAPTER_H
#define BUSADAPTER_H

#include <QObject>
#include <QPointer>

#include "Bus.h"
#include "GrpcClient.h"

class BusAdapter : public QObject
{
    Q_OBJECT
  public:
    static QPointer<BusAdapter> instance()
    {
        static QPointer<BusAdapter> inst = new BusAdapter();
        return inst;
    }

  signals:
    void signalPing();
    void signalModelInfoUpdateNtf(const QVector<Bus::ModelInfo> &configs);
    void signalMemoryInfoUpdateNtf(const QVector<Bus::MemoryInfo> &configs);
    void signalAudioParamUpdateNtf(const QVector<Bus::AudioParam> &params);

  private slots:
    // for BUS signals
    void _slotPong();
    void _slotNewSessionFromBus(const QString &title,
                                const QString &content,
                                const QString &model);
    void _slotGetSessionFromBus(const int64_t sessionId, int limit);
    void _slotQueryFromBus(const int64_t         sessionId,
                           const int64_t         msgId,
                           const QString        &query,
                           const QString        &model,
                           const Bus::ModelInfo &config);
    void _slotStopAnswerFromBus(const int64_t sessionId);
    void _slotGetChatMessageFromBus(const int64_t msgId,
                                    const int64_t sessionId,
                                    int           limit);
    void _slotDelSessionFromBus(const QVector<int64_t> &ids);
    void _slotAudioTranslate(const qint64      sessionId,
                             const QByteArray &src,
                             const QString    &translatorId);
    void _slotAudioStopTranslate(const qint64 sessionId);
    void _slotUploadFromBus(const QString &filePath);
    void _slotRetrieveFromBus(const QString &question,
                              const int      topK,
                              const QString &memoryId);
    void _slotEmbeddingFromBus(const QStringList &files,
                               const QString     &memoryId);

    // for framework signals:
    void _slotEmbeddingProgress(const int64_t     taskId,
                                const int64_t     chunkId,
                                const QString    &memoryId,
                                const int         totalChunkNum,
                                const int         finishedChunkNum,
                                const QByteArray &vectorIndexs);
    void _slotEmbeddingFinished(const int      errorCode,
                                const int64_t  taskId,
                                const QString &memoryId);
    void _slotRetrieveFinished(const int             errorCode,
                               const int64_t         taskId,
                               const QString        &text,
                               const int             topK,
                               const QString        &memoryId,
                               QVector<QJsonObject> &memorys);

  private:
    explicit BusAdapter(QObject *parent = nullptr);
    ~BusAdapter();

    int64_t m_currRetrieveTaskId = -1;
    int64_t m_currEmbTaskId      = -1;
};

#endif // BUSADAPTER_H