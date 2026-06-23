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
    void SignalModelInfoUpdate(const QVector<Bus::Model> &modelInfos);

  private slots:
    // for BUS signals
    void _slotPong();
    void _slotNewSessionFromBus(const QString &title,
                                const QString &content,
                                const QString &model);
    void _slotGetSessionFromBus(const int64_t sessionId, int limit);
    void _slotQueryFromBus(const int64_t  sessionId,
                           const QString &query,
                           const QString &model);
    void _slotGetMessageInfoFromBus(const int64_t msgId,
                                    const int64_t sessionId,
                                    int           limit);

  private:
    explicit BusAdapter(QObject *parent = nullptr);
    ~BusAdapter();

    static BusAdapter *m_stBusAdapterInst;
};

#endif // BUSADAPTER_H