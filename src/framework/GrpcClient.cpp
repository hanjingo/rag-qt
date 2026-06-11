#include "GrpcClient.h"

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

void GrpcClient::connect(const QString &address)
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

void GrpcClient::query(const QString &content)
{
    if(!m_pChannel)
    {
        emit SignalQueryResp("Not connected to gRPC server");
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::QueryReq req;
    req.set_content(content.toStdString());

    // Prepare the response and context
    GrpcLibrary::QueryResp resp;
    grpc::ClientContext    context;

    // Make the RPC call
    grpc::Status status = stub->Query(&context, req, &resp);

    if(status.ok())
        emit SignalQueryResp(QString::fromStdString(resp.content()));
    else
        emit SignalQueryResp(QString::fromStdString(status.error_message()));
}

void GrpcClient::get_history(const QString &id)
{
    QVector<History> ret;
    if(!m_pChannel)
    {
        emit SignalGetHistoryResp(ret);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::GetHistoryReq req;
    req.set_id(id.toInt());

    // Prepare the response and context
    GrpcLibrary::GetHistoryResp resp;
    grpc::ClientContext         context;

    // Make the RPC call
    grpc::Status status = stub->GetHistory(&context, req, &resp);

    if(!status.ok())
    {
        emit SignalGetHistoryResp(ret);
        return;
    }

    for(int i = 0; i < resp.history_size(); i++)
    {
        const auto &h = resp.history(i);
        History     history;
        history.datetime = QString::fromStdString(h.datetime());
        history.content  = QString::fromStdString(h.content());
        ret.append(history);
    }
    emit SignalGetHistoryResp(ret);
}