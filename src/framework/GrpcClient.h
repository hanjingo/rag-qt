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

    static GrpcClient *Instance();

    void Connect(const QString &address);
    void Login(const QString &username, const QString &password);
    void Logout(const int32_t user_id, const QString &auth);
    void RegAccount(const QString &username, const QString &password);
    void Query(const int64_t  id,
               const int32_t  user_id,
               const QString &auth,
               const QString &content,
               const QString &model);
    void GetSession(const int64_t  id,
                    const int32_t  user_id,
                    const QString &auth,
                    int            limit = 10);
    void NewSession(const int32_t  user_id,
                    const QString &auth,
                    const QString &title,
                    const QString &content = "",
                    const QString &model   = "");
    void ModifySessionTitle(const int32_t  user_id,
                            const QString &auth,
                            const int64_t  id,
                            const QString &title);

    void GetSkillInfo(const int64_t id = -1, int limit = 50);

    void
    Download(const QString &hash, const int32_t user_id, const QString &auth);

  signals:
    void SignalGrpcConnected(const QString &address);
    void SignalGrpcConnectFailed(const QString &address);
    void SignalLoginResp(const int      errorCode,
                         const int32_t  id,
                         const QString &auth,
                         const int32_t  privilege,
                         const QString &account,
                         const QString &lastLoginTime);
    void SignalRegAccountResp(const int errorCode, const int32_t user_id);
    void SignalQueryResp(const int      errorCode,
                         const int64_t  id,
                         const QString &content);
    void SignalLogoutResp(const int errorCode, const int user_id);
    void SignalGetSessionResp(const int                              errorCode,
                              const QVector<::GrpcLibrary::Session> &sessions);
    void SignalNewSessionResp(const int                     errorCode,
                              const ::GrpcLibrary::Session &session);
    void SignalModifySessionTitleResp(const int      errorCode,
                                      const int64_t  id,
                                      const QString &title);

    void SignalGetSkillInfoResp(const int                            errorCode,
                                const QVector<::GrpcLibrary::Skill> &skills);
    void SignalDownloadResp(const int      errorCode,
                            const QString &hash,
                            const QString &addr,
                            const int64_t  size_kb);

  private:
    static GrpcClient *m_stGrpcClientInst;
    hj::grpc_channel  *m_pChannel;
};

#endif