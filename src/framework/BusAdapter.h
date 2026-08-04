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
    void signalRetrieveResp(const int                   errorCode,
                            const QString              &question,
                            const int                   topK,
                            const QString              &memoryId,
                            const QVector<QJsonObject> &memorys);

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
    void _slotGetMessageInfoFromBus(const int64_t msgId,
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

  private:
    explicit BusAdapter(QObject *parent = nullptr);
    ~BusAdapter();
};

#endif // BUSADAPTER_H