#ifndef BUSADAPTER_H
#define BUSADAPTER_H

#include <QObject>

#include "Bus.h"
#include "GrpcClient.h"

class BusAdapter : public QObject
{
    Q_OBJECT
  public:
    static BusAdapter *Instance();

  signals:
    void SignalPing();
    void SignalModelInfoUpdateNtf(const QVector<Bus::ModelInfo> &configs);
    void SignalAudioParamUpdateNtf(const QVector<Bus::AudioParam> &params);

  private slots:
    // for BUS signals
    void _slotPong();
    void _slotNewSessionFromBus(const QString &title,
                                const QString &content,
                                const QString &model);
    void _slotGetSessionFromBus(const int64_t sessionId, int limit);
    void _slotQueryFromBus(const int64_t         sessionId,
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

  private:
    explicit BusAdapter(QObject *parent = nullptr);
    ~BusAdapter();

    static BusAdapter *m_stBusAdapterInst;
};

#endif // BUSADAPTER_H