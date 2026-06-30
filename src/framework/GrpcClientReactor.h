#ifndef GRPCCLIENTREACTOR_H
#define GRPCCLIENTREACTOR_H

#include <hj/net/grpc.hpp>
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

#endif // GRPCCLIENTREACTOR_H