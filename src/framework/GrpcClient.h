#ifndef GRPCCLIENT_H
#define GRPCCLIENT_H

#include <atomic>

#include <QObject>
#include <QString>
#include <QVector>
#include <QPointer>

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

    static QPointer<GrpcClient> instance()
    {
        static QPointer<GrpcClient> inst = new GrpcClient();
        return inst;
    }

    void RemoveRecognizeReactor(int64_t sessionId);
    void RemoveEmbeddingReactor(int64_t taskId);

    QString Addr() { return m_strAddress; }
    bool    IsConnected() const { return m_bIsConnected.load(); }
    void    Connect(const QString &address);
    void    Heartbeat(const int64_t timestamp);
    void    Login(const QString &username, const QString &password);
    void    Logout(const int64_t user_id, const QString &auth);
    void    RegAccount(const QString &username, const QString &password);

    void Query(const int64_t              id,
               const int64_t              msgId,
               const int64_t              userId,
               const QString             &auth,
               const QString             &content,
               const QString             &model,
               const Config::ModelConfig &config);
    void StopAnswer(const int64_t  session_id,
                    const int64_t  user_id,
                    const QString &auth);
    void GetChatMessage(const int64_t  session_id,
                        const int64_t  user_id,
                        const QString &auth,
                        int64_t        msg_id = -1,
                        int            limit  = 10);

    void Recognize(const int64_t           session_id,
                   const int64_t           user_id,
                   const QString          &auth,
                   const QByteArray       &data,
                   const Config::AsrParam &params,
                   const QString          &translatorId);

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

    void Publish(const int64_t           user_id,
                 const QString          &auth,
                 const QVector<QString> &msgs);

    void Subscribe(const int64_t           user_id,
                   const QString          &auth,
                   const QVector<QString> &topics);

    void UnSubscribe(const int64_t           user_id,
                     const QString          &auth,
                     const QVector<QString> &topics);


  signals:
    void signalGrpcConnected(const QString &address);
    void signalGrpcConnectFailed(const QString &address);
    void signalGrpcDisconnected(const QString &address);
    void signalPong(const int64_t timestamp);
    void signalLoginResp(const int      errorCode,
                         const int64_t  user_id,
                         const QString &auth,
                         const QString &account,
                         const QString &lastLoginTime,
                         const bool     isForceUpdate);
    void signalRegAccountResp(const int errorCode, const int64_t user_id);

    void signalQueryResp(const int      errorCode,
                         const int64_t  sessionId,
                         const int64_t  msgId,
                         const QString &content,
                         const bool     isFinished);
    void signalStopAnswerResp(const int64_t errorCode, const int64_t sessionId);
    void signalGetChatMessageResp(const int                        errorCode,
                                  const QVector<Bus::ChatMessage> &messages);

    void signalRecognizeResp(const int      errorCode,
                             const QString &transcript,
                             const bool     isFinished,
                             const double   confidence);
    void signalStopRecognizeResp(const int errorCode, const int64_t sessionId);

    void signalEmbeddingResp(const int         errorCode,
                             const int64_t     taskId,
                             const int64_t     chunkId,
                             const QByteArray &vectorIndexs);
    void signalStopEmbeddingResp(const int errorCode, const int64_t taskId);

    void signalLogoutResp(const int errorCode, const int64_t user_id);
    void signalGetSessionResp(const int                    errorCode,
                              const QVector<Bus::Session> &sessions);
    void signalNewSessionResp(const int errorCode, const Bus::Session &session);
    void signalModifySessionTitleResp(const int      errorCode,
                                      const int64_t  id,
                                      const QString &title);
    void signalDelSessionResp(const int errorCode, const QVector<int64_t> &ids);

    void signalGetPluginInfoResp(const int                   errorCode,
                                 const QVector<Bus::Plugin> &plugins);
    void signalDownloadResp(const int      errorCode,
                            const QString &hash,
                            const QString &addr,
                            const int64_t  size_kb);
    void signalUploadResp(const int errorCode, const QString &hash);

    void signalPublishResp(const int errorCode);

    void signalPubMessageNtf(const QString &topic, const QString &content);

    void signalUnSubscribeResp(const int               errorCode,
                               const QVector<QString> &unsubscribedTopics);

  public slots:
    void OnConnectionLost();

  private:
    void _convert(::GrpcLibraryV1::Session &dst, const Bus::Session &src);
    void _convert(Bus::Session &dst, const ::GrpcLibraryV1::Session &src);
    void _convert(::GrpcLibraryV1::Plugin &dst, const Bus::Plugin &src);
    void _convert(Bus::Plugin &dst, const ::GrpcLibraryV1::Plugin &src);
    void _convert(::GrpcLibraryV1::ChatMessage &dst,
                  const Bus::ChatMessage       &src);
    void _convert(Bus::ChatMessage                   &dst,
                  const ::GrpcLibraryV1::ChatMessage &src);

  private:
    hj::grpc_channel *m_pChannel;
    std::atomic<bool> m_bIsConnected;
    QString           m_strAddress;
};

#endif