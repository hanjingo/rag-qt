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
#include "Config.h"

class RecognizeReactor;

class GrpcClient : public QObject
{
    Q_OBJECT

  public:
    explicit GrpcClient(QObject *parent = nullptr);
    ~GrpcClient();

    static GrpcClient *Instance();

    void RemoveRecognizeReactor(int64_t sessionId);
    void RemoveEmbeddingReactor(int64_t taskId);

    bool IsConnected() const { return m_bIsConnected.load(); }
    void Connect(const QString &address);
    void Heartbeat(const int64_t timestamp);
    void Login(const QString &username, const QString &password);
    void Logout(const int64_t user_id, const QString &auth);
    void RegAccount(const QString &username, const QString &password);

    void Query(const int64_t              id,
               const int64_t              user_id,
               const QString             &auth,
               const QString             &content,
               const QString             &model,
               const Config::ModelConfig &config);
    void StopAnswer(const int64_t  session_id,
                    const int64_t  user_id,
                    const QString &auth);
    void GetMessageInfo(const int64_t  session_id,
                        const int64_t  user_id,
                        const QString &auth,
                        int64_t        msg_id = -1,
                        int            limit  = 10);

    void Recognize(const int64_t                  session_id,
                   const int64_t                  user_id,
                   const QString                 &auth,
                   const QByteArray              &data,
                   const Config::TranslatorParam &params,
                   const QString                 &translatorId);

    void RecognizeStop(const int64_t  session_id,
                       const int64_t  user_id,
                       const QString &auth);

    void Embedding(const int64_t               task_id,
                   const int64_t               user_id,
                   const QString              &auth,
                   const int64_t               chunk_id,
                   const QByteArray           &chunk_data,
                   const int64_t               start_pos,
                   const int64_t               end_pos,
                   const Config::MemoryConfig &params = Config::MemoryConfig());
    void EmbeddingStop(const int64_t  task_id,
                       const int64_t  user_id,
                       const QString &auth);

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

    void GetPluginInfo(const QString &hash      = "",
                       const QString &publisher = "",
                       int            limit     = 50);

    void
    Download(const QString &hash, const int64_t user_id, const QString &auth);

    void Upload(const QString &hash,
                const int64_t  user_id,
                const QString &auth,
                const QString &addr,
                const int64_t  size_kb);

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

    void SignalRecognizeResp(const int      errorCode,
                             const QString &transcript,
                             const bool     isFinished,
                             const double   confidence);
    void SignalStopRecognizeResp(const int errorCode, const int64_t sessionId);

    void SignalEmbeddingResp(const int         errorCode,
                             const int64_t     taskId,
                             const int64_t     chunkId,
                             const QByteArray &vectorIndexs);
    void SignalStopEmbeddingResp(const int errorCode, const int64_t taskId);

    void SignalLogoutResp(const int errorCode, const int64_t user_id);
    void SignalGetSessionResp(const int                    errorCode,
                              const QVector<Bus::Session> &sessions);
    void SignalNewSessionResp(const int errorCode, const Bus::Session &session);
    void SignalModifySessionTitleResp(const int      errorCode,
                                      const int64_t  id,
                                      const QString &title);
    void SignalDelSessionResp(const int errorCode, const QVector<int64_t> &ids);

    void SignalGetPluginInfoResp(const int                   errorCode,
                                 const QVector<Bus::Plugin> &plugins);
    void SignalDownloadResp(const int      errorCode,
                            const QString &hash,
                            const QString &addr,
                            const int64_t  size_kb);
    void SignalUploadResp(const int errorCode, const QString &hash);

  public slots:
    void OnConnectionLost();

  private:
    void _convert(::GrpcLibrary::Session &dst, const Bus::Session &src);
    void _convert(Bus::Session &dst, const ::GrpcLibrary::Session &src);
    void _convert(::GrpcLibrary::Plugin &dst, const Bus::Plugin &src);
    void _convert(Bus::Plugin &dst, const ::GrpcLibrary::Plugin &src);
    void _convert(::GrpcLibrary::MessageInfo &dst, const Bus::MessageInfo &src);
    void _convert(Bus::MessageInfo &dst, const ::GrpcLibrary::MessageInfo &src);

  private:
    static GrpcClient *m_stGrpcClientInst;
    hj::grpc_channel  *m_pChannel;

    std::atomic<bool> m_bIsConnected;
    QString           m_strAddress;
};

#endif