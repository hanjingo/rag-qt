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
    explicit GrpcClient(QObject *parent = nullptr);
    ~GrpcClient();

    static GrpcClient *GetGrpcClientInst();

    void Connect(const QString &address);
    void Query(const int64_t  id,
               const int32_t  user_id,
               const QString &auth,
               const QString &content);
    void GetSession(const int64_t  id,
                    const int32_t  user_id,
                    const QString &auth,
                    int            limit = 10);
    void NewSession(const int32_t  user_id,
                    const QString &auth,
                    const QString &title);
    void ModifySessionTitle(const int32_t  user_id,
                            const QString &auth,
                            const int64_t  id,
                            const QString &title);

  signals:
    void SignalGrpcConnected(const QString &address);
    void SignalGrpcConnectFailed(const QString &address);
    void SignalQueryResp(const int      errorCode,
                         const int64_t  id,
                         const QString &content);
    void SignalGetSessionResp(const int                              errorCode,
                              const QVector<::GrpcLibrary::Session> &sessions);
    void SignalNewSessionResp(const int                     errorCode,
                              const ::GrpcLibrary::Session &session);
    void SignalModifySessionTitleResp(const int      errorCode,
                                      const int64_t  id,
                                      const QString &title);

  private:
    static GrpcClient *m_stGrpcClientInst;
    hj::grpc_channel  *m_pChannel;
};

#endif