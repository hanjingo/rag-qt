#ifndef GRPCCLIENTREACTOR_H
#define GRPCCLIENTREACTOR_H

#include <hj/net/grpc.hpp>
#include <hj/sync/channel.hpp>

#include "src/api.grpc.pb.h"
#include "GrpcClient.h"

// async query reactor
class QueryReactor : public grpc::ClientReadReactor<GrpcLibraryV1::QueryResp>
{
  public:
    QueryReactor(QPointer<GrpcClient> client, int64_t id)
        : m_client(client)
        , m_id(id)
    {
        qDebug() << "QueryReactor created for id:" << id;
    }

    void OnReadDone(bool ok) override;
    void OnDone(const grpc::Status &status) override;

    GrpcLibraryV1::QueryResp m_resp;
    grpc::ClientContext      m_context;

  private:
    QPointer<GrpcClient> m_client;
    int64_t              m_id;
};

// async recognize reactor
class RecognizeReactor
    : public grpc::ClientBidiReactor<GrpcLibraryV1::RecognizeReq,
                                     GrpcLibraryV1::RecognizeResp>,
      public std::enable_shared_from_this<RecognizeReactor>
{
  public:
    RecognizeReactor(QPointer<GrpcClient> client, int64_t sessionId);
    ~RecognizeReactor();

    void OnReadDone(bool ok) override;
    void OnWriteDone(bool ok) override;
    void OnDone(const grpc::Status &status) override;

    void OnConnectionLost();

    void SendRequest(const GrpcLibraryV1::RecognizeReq &req);

    int64_t GetSessionId() const { return m_sessionId; }
    bool    IsDone() const { return m_isDone.load(); }

    grpc::ClientContext          m_context;
    GrpcLibraryV1::RecognizeResp m_resp;

  private:
    void _flush();
    void _pull();

  private:
    int64_t              m_sessionId;
    QPointer<GrpcClient> m_client;

    hj::channel<GrpcLibraryV1::RecognizeReq> m_writeCh;

    std::atomic<bool> m_isWriting{false};
    std::atomic<bool> m_isDone{false};
};

// async embedding reactor
class EmbeddingReactor
    : public grpc::ClientBidiReactor<GrpcLibraryV1::EmbeddingReq,
                                     GrpcLibraryV1::EmbeddingResp>,
      public std::enable_shared_from_this<EmbeddingReactor>
{
  public:
    EmbeddingReactor(QPointer<GrpcClient> client, int64_t taskId);
    ~EmbeddingReactor();

    void OnReadDone(bool ok) override;
    void OnWriteDone(bool ok) override;
    void OnDone(const grpc::Status &status) override;

    void OnConnectionLost();

    void SendRequest(const GrpcLibraryV1::EmbeddingReq &req);

    int64_t GetTaskId() const { return m_taskId; }
    bool    IsDone() const { return m_isDone.load(); }

    grpc::ClientContext          m_context;
    GrpcLibraryV1::EmbeddingResp m_resp;

  private:
    void _flush();
    void _pull();

  private:
    int64_t              m_taskId;
    QPointer<GrpcClient> m_client;

    hj::channel<GrpcLibraryV1::EmbeddingReq> m_writeCh;

    std::atomic<bool> m_isWriting{false};
    std::atomic<bool> m_isReading{false};
    std::atomic<bool> m_isDone{false};
};

#endif // GRPCCLIENTREACTOR_H