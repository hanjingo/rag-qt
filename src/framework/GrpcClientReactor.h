#ifndef GRPCCLIENTREACTOR_H
#define GRPCCLIENTREACTOR_H

#include <hj/net/grpc.hpp>
#include <hj/sync/channel.hpp>

#include "src/api.grpc.pb.h"
#include "GrpcClient.h"

// async query reactor
class QueryReactor : public grpc::ClientReadReactor<GrpcLibrary::QueryResp>
{
  public:
    QueryReactor(GrpcClient *client, int64_t id)
        : m_client(client)
        , m_id(id)
    {
    }

    void OnReadDone(bool ok) override;
    void OnDone(const grpc::Status &status) override;

    GrpcLibrary::QueryResp m_resp;
    grpc::ClientContext    m_context;

  private:
    GrpcClient *m_client;
    int64_t     m_id;
};

// async recognize reactor
class RecognizeReactor
    : public grpc::ClientBidiReactor<GrpcLibrary::RecognizeReq,
                                     GrpcLibrary::RecognizeResp>,
      public std::enable_shared_from_this<RecognizeReactor>
{
  public:
    RecognizeReactor(GrpcClient *client, int64_t sessionId);
    ~RecognizeReactor();

    void OnReadDone(bool ok) override;
    void OnWriteDone(bool ok) override;
    void OnDone(const grpc::Status &status) override;

    void OnConnectionLost();

    void SendRequest(const GrpcLibrary::RecognizeReq &req);

    int64_t GetSessionId() const { return m_sessionId; }
    bool    IsDone() const { return m_isDone.load(); }

    grpc::ClientContext        m_context;
    GrpcLibrary::RecognizeResp m_resp;

  private:
    void _flush();
    void _pull();

  private:
    int64_t     m_sessionId;
    GrpcClient *m_client;

    hj::channel<GrpcLibrary::RecognizeReq> m_writeCh;

    std::atomic<bool> m_isWriting{false};
    std::atomic<bool> m_isDone{false};
};

// async embedding reactor
class EmbeddingReactor
    : public grpc::ClientBidiReactor<GrpcLibrary::EmbeddingReq,
                                     GrpcLibrary::EmbeddingResp>,
      public std::enable_shared_from_this<EmbeddingReactor>
{
  public:
    EmbeddingReactor(GrpcClient *client, int64_t taskId);
    ~EmbeddingReactor();

    void OnReadDone(bool ok) override;
    void OnWriteDone(bool ok) override;
    void OnDone(const grpc::Status &status) override;

    void OnConnectionLost();

    void SendRequest(const GrpcLibrary::EmbeddingReq &req);

    int64_t GetTaskId() const { return m_taskId; }
    bool    IsDone() const { return m_isDone.load(); }

    grpc::ClientContext        m_context;
    GrpcLibrary::EmbeddingResp m_resp;

  private:
    void _flush();
    void _pull();

  private:
    int64_t     m_taskId;
    GrpcClient *m_client;

    hj::channel<GrpcLibrary::EmbeddingReq> m_writeCh;

    std::atomic<bool> m_isWriting{false};
    std::atomic<bool> m_isDone{false};
};

#endif // GRPCCLIENTREACTOR_H