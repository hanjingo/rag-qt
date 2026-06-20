#include "GrpcClient.h"

#include "Error.h"

GrpcClient *GrpcClient::m_stGrpcClientInst = nullptr;
GrpcClient *GrpcClient::Instance()
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

void GrpcClient::Login(const QString &username, const QString &password)
{
    if(!m_pChannel)
    {
        emit SignalLoginResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                             -1,
                             "",
                             -1,
                             username,
                             "");
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::LoginReq req;
    req.set_account(username.toStdString());
    req.set_passwd(password.toStdString());

    // Prepare the response and context
    GrpcLibrary::LoginResp resp;
    grpc::ClientContext    context;

    // Make the RPC call
    grpc::Status status = stub->Login(&context, req, &resp);

    if(status.ok())
        emit SignalLoginResp(resp.error_code(),
                             resp.user_id(),
                             QString::fromStdString(resp.auth()),
                             resp.privilege(),
                             QString::fromStdString(resp.account()),
                             QString::fromStdString(resp.last_login_time()));
    else
        emit SignalLoginResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                             -1,
                             "",
                             -1,
                             username,
                             "");
}

void GrpcClient::Logout(const int32_t user_id, const QString &auth)
{
    if(!m_pChannel)
    {
        emit SignalLogoutResp(ErrorCode::ERR_SERVER_DISCONNECTED, -1);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::LogoutReq req;
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());

    // Prepare the response and context
    GrpcLibrary::LogoutResp resp;
    grpc::ClientContext     context;

    // Make the RPC call
    grpc::Status status = stub->Logout(&context, req, &resp);

    if(status.ok())
        emit SignalLogoutResp(resp.error_code(), resp.user_id());
    else
        emit SignalLogoutResp(ErrorCode::ERR_SERVER_DISCONNECTED, -1);
}

void GrpcClient::RegAccount(const QString &username, const QString &password)
{
    if(!m_pChannel)
    {
        emit SignalRegAccountResp(ErrorCode::ERR_SERVER_DISCONNECTED, -1);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::RegAccountReq req;
    req.set_account(username.toStdString());
    req.set_passwd(password.toStdString());

    // Prepare the response and context
    GrpcLibrary::RegAccountResp resp;
    grpc::ClientContext         context;

    // Make the RPC call
    grpc::Status status = stub->RegAccount(&context, req, &resp);

    if(status.ok())
        emit SignalRegAccountResp(resp.error_code(), resp.user_id());
    else
        emit SignalRegAccountResp(ErrorCode::ERR_SERVER_DISCONNECTED, -1);
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
        emit SignalQueryResp(resp.error_code(),
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
    emit SignalGetSessionResp(resp.error_code(), ret);
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

    emit SignalNewSessionResp(resp.error_code(), resp.session());
}

void GrpcClient::ModifySessionTitle(const int32_t  user_id,
                                    const QString &auth,
                                    const int64_t  id,
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

    emit SignalModifySessionTitleResp(resp.error_code(), id, title);
}

void GrpcClient::GetSkillInfo(const int64_t id, int limit)
{
    if(!m_pChannel)
    {
        emit SignalGetSkillInfoResp(ErrorCode::ERR_SERVER_DISCONNECTED, {});
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::GetSkillInfoReq req;
    req.set_id(id);
    req.set_limit(limit);

    // Prepare the response and context
    GrpcLibrary::GetSkillInfoResp resp;
    grpc::ClientContext           context;

    // Make the RPC call
    grpc::Status status = stub->GetSkillInfo(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit SignalGetSkillInfoResp(ec, {});
        return;
    }

    QVector<::GrpcLibrary::Skill> ret;
    for(int i = 0; i < resp.skills_size(); i++)
    {
        const auto &h = resp.skills(i);
        ret.append(h);
    }
    emit SignalGetSkillInfoResp(resp.error_code(), ret);
}

void GrpcClient::Download(const QString &hash,
                          const int32_t  user_id,
                          const QString &auth)
{
    if(!m_pChannel)
    {
        emit SignalDownloadResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                hash,
                                "",
                                0);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::DownloadReq req;
    req.set_hash(hash.toStdString());
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());

    // Prepare the response and context
    GrpcLibrary::DownloadResp resp;
    grpc::ClientContext       context;

    // Make the RPC call
    grpc::Status status = stub->Download(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit SignalDownloadResp(ec, hash, "", 0);
        return;
    }

    QString addr = QString::fromStdString(resp.addr());
    emit    SignalDownloadResp(resp.error_code(), hash, addr, resp.size_kb());
}