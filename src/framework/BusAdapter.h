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


  private slots:
    // for BUS signals
    void _slotPong();
    void _slotNewSession(const QString &title,
                         const QString &content,
                         const QString &model);
    void _slotNewSessionResp(const int                     errorCode,
                             const ::GrpcLibrary::Session &session);
    void _slotQuery(const int64_t  sessionId,
                    const QString &query,
                    const QString &model);
    void _slotQueryResp(const int      errorCode,
                        const int64_t  id,
                        const QString &content);

  private:
    explicit BusAdapter(QObject *parent = nullptr);
    ~BusAdapter();

    static BusAdapter *m_stBusAdapterInst;
};

#endif // BUSADAPTER_H