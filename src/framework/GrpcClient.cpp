#include "GrpcClient.h"

#include "Error.h"

GrpcClient *GrpcClient::m_stGrpcClientInst = nullptr;
GrpcClient *GrpcClient::GetGrpcClientInst()
{
    if(nullptr == m_stGrpcClientInst)
        m_stGrpcClientInst = new GrpcClient();

    return m_stGrpcClientInst;
}

GrpcClient::GrpcClient(QObject *parent)
    : QObject(parent)
    , m_pChannel(nullptr)
{
}

GrpcClient::~GrpcClient()
{
    if(m_pChannel)
    {
        delete m_pChannel;
        m_pChannel = nullptr;
    }
}

void GrpcClient::Connect(const QString &address)
{
    if(m_pChannel)
    {
        delete m_pChannel;
        m_pChannel = nullptr;
    }
    m_pChannel = new hj::grpc_channel();
    if(m_pChannel->connect(address.toStdString()))
        emit SignalGrpcConnected(address);
    else
        emit SignalGrpcConnectFailed(address);
}

void GrpcClient::Query(const int64_t  id,
                       const int32_t  user_id,
                       const QString &auth,
                       const QString &content)
{
    if(!m_pChannel)
    {
        emit SignalQueryResp(ErrorCode::ERR_SERVER_DISCONNECTED, id, "");
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::QueryReq req;
    req.set_id(id);
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    req.set_content(content.toStdString());

    // Prepare the response and context
    GrpcLibrary::QueryResp resp;
    grpc::ClientContext    context;

    // Make the RPC call
    grpc::Status status = stub->Query(&context, req, &resp);

    if(status.ok())
        emit SignalQueryResp(ErrorCode::OK,
                             id,
                             QString::fromStdString(resp.content()));
    else
        emit SignalQueryResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                             id,
                             QString::fromStdString(status.error_message()));
}

void GrpcClient::GetSession(const int64_t  id,
                            const int32_t  user_id,
                            const QString &auth,
                            int            limit)
{
    QVector<::GrpcLibrary::Session> ret;
    if(!m_pChannel)
    {
        emit SignalGetSessionResp(ErrorCode::ERR_SERVER_DISCONNECTED, ret);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::GetSessionReq req;
    req.set_id(id);
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    req.set_limit(limit);

    // Prepare the response and context
    GrpcLibrary::GetSessionResp resp;
    grpc::ClientContext         context;

    // Make the RPC call
    grpc::Status status = stub->GetSession(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit SignalGetSessionResp(ec, ret);
        return;
    }

    for(int i = 0; i < resp.sessions_size(); i++)
    {
        const auto &h = resp.sessions(i);
        ret.append(h);
    }
    emit SignalGetSessionResp(ErrorCode::OK, ret);
}

void GrpcClient::NewSession(const int32_t  user_id,
                            const QString &auth,
                            const QString &title)
{
    if(!m_pChannel)
    {
        emit SignalNewSessionResp(ErrorCode::ERR_SERVER_DISCONNECTED, {});
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::NewSessionReq req;
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    req.set_title(title.toStdString());

    // Prepare the response and context
    GrpcLibrary::NewSessionResp resp;
    grpc::ClientContext         context;

    // Make the RPC call
    grpc::Status status = stub->NewSession(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit SignalNewSessionResp(ec, {});
        return;
    }

    emit SignalNewSessionResp(ErrorCode::OK, resp.session());
}

void GrpcClient::ModifySessionTitle(const int32_t  user_id,
                                    const QString &auth,
                                    const int64_t id,
                                    const QString &title)
{
    if(!m_pChannel)
    {
        emit SignalModifySessionTitleResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                          id,
                                          title);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::ModifySessionTitleReq req;
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    req.set_id(id);
    req.set_title(title.toStdString());

    // Prepare the response and context
    GrpcLibrary::ModifySessionTitleResp resp;
    grpc::ClientContext                 context;

    // Make the RPC call
    grpc::Status status = stub->ModifySessionTitle(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit SignalModifySessionTitleResp(ec, id, title);
        return;
    }

    emit SignalModifySessionTitleResp(ErrorCode::OK, id, title);
}