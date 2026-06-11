#ifndef GRPCCLIENT_H
#define GRPCCLIENT_H

#include <QObject>
#include <QString>
#include <QVector>

#include <hj/net/grpc.hpp>
#include "src/api.grpc.pb.h"

class GrpcClient : public QObject
{
    Q_OBJECT
  public:
    struct History
    {
        QString datetime;
        QString content;
    };


  public:
    explicit GrpcClient(QObject *parent = nullptr);
    ~GrpcClient();

    static GrpcClient *GetGrpcClientInst();
    void               connect(const QString &address);
    void               query(const QString &content);
    void               get_history(const QString &id);

  signals:
    void SignalGrpcConnected(const QString &address);
    void SignalGrpcConnectFailed(const QString &address);
    void SignalQueryResp(const QString &resp);
    void SignalGetHistoryResp(const QVector<History> &resp);

  private:
    static GrpcClient *m_stGrpcClientInst;
    hj::grpc_channel  *m_pChannel;
};

#endif