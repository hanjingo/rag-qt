#ifndef GRPCCLIENT_H
#define GRPCCLIENT_H

#include <atomic>

#include <QObject>
#include <QString>
#include <QVector>

#include <hj/net/grpc.hpp>
#include "src/api.grpc.pb.h"
#include "Bus.h"
#include "Global.h"

class GrpcClient : public QObject
{
    Q_OBJECT

  public:
    explicit GrpcClient(QObject *parent = nullptr);
    ~GrpcClient();

    static GrpcClient *Instance();

    bool IsConnected() const { return m_bIsConnected.load(); }

    void Connect(const QString &address);
    void Heartbeat(const int64_t timestamp);
    void Login(const QString &username, const QString &password);
    void Logout(const int64_t user_id, const QString &auth);
    void RegAccount(const QString &username, const QString &password);

    void Query(const int64_t           id,
               const int64_t           user_id,
               const QString          &auth,
               const QString          &content,
               const QString          &model,
               const Bus::ModelConfig &config);
    void StopAnswer(const int64_t  session_id,
                    const int64_t  user_id,
                    const QString &auth);
    void GetMessageInfo(const int64_t  session_id,
                        const int64_t  user_id,
                        const QString &auth,
                        int64_t        msg_id = -1,
                        int            limit  = 10);

    void GetSession(const int64_t  id,
                    const int64_t  user_id,
                    const QString &auth,
                    int            limit = 10);
    void NewSession(const int64_t  user_id,
                    const QString &auth,
                    const QString &title,
                    const QString &prompt = "",
                    const QString &model  = "");
    void ModifySessionTitle(const int64_t  user_id,
                            const QString &auth,
                            const int64_t  id,
                            const QString &title);
    void DelSession(const int64_t           user_id,
                    const QString          &auth,
                    const QVector<int64_t> &ids);

    void GetSkillInfo(const QString &hash = "", int limit = 50);

    void
    Download(const QString &hash, const int64_t user_id, const QString &auth);

  signals:
    void SignalGrpcConnected(const QString &address);
    void SignalGrpcConnectFailed(const QString &address);
    void SignalGrpcDisconnected(const QString &address);
    void SignalPong(const int64_t timestamp);
    void SignalLoginResp(const int      errorCode,
                         const int64_t  user_id,
                         const QString &auth,
                         const int32_t  privilege,
                         const QString &account,
                         const QString &lastLoginTime);
    void SignalRegAccountResp(const int errorCode, const int64_t user_id);

    void SignalQueryResp(const int      errorCode,
                         const int64_t  sessionId,
                         const QString &content,
                         const bool     isFinished);
    void SignalStopAnswerResp(const int64_t errorCode, const int64_t sessionId);
    void SignalGetMessageInfoResp(const int                        errorCode,
                                  const QVector<Bus::MessageInfo> &messages);

    void SignalLogoutResp(const int errorCode, const int64_t user_id);
    void SignalGetSessionResp(const int                    errorCode,
                              const QVector<Bus::Session> &sessions);
    void SignalNewSessionResp(const int errorCode, const Bus::Session &session);
    void SignalModifySessionTitleResp(const int      errorCode,
                                      const int64_t  id,
                                      const QString &title);
    void SignalDelSessionResp(const int errorCode, const QVector<int64_t> &ids);

    void SignalGetSkillInfoResp(const int                  errorCode,
                                const QVector<Bus::Skill> &skills);
    void SignalDownloadResp(const int      errorCode,
                            const QString &hash,
                            const QString &addr,
                            const int64_t  size_kb);

  private:
    void _convert(::GrpcLibrary::Session &dst, const Bus::Session &src);
    void _convert(Bus::Session &dst, const ::GrpcLibrary::Session &src);
    void _convert(::GrpcLibrary::Skill &dst, const Bus::Skill &src);
    void _convert(Bus::Skill &dst, const ::GrpcLibrary::Skill &src);
    void _convert(::GrpcLibrary::MessageInfo &dst, const Bus::MessageInfo &src);
    void _convert(Bus::MessageInfo &dst, const ::GrpcLibrary::MessageInfo &src);

  private:
    static GrpcClient *m_stGrpcClientInst;
    hj::grpc_channel  *m_pChannel;

    std::atomic<bool> m_bIsConnected;
    QString           m_strAddress;
};

#endif