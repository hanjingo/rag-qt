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

void GrpcClient::Logout(const int64_t user_id, const QString &auth)
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
                       const int64_t  user_id,
                       const QString &auth,
                       const QString &content,
                       const QString &model)
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
    req.set_model(model.toStdString());

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

void GrpcClient::GetMessageInfo(const int64_t  session_id,
                                const int64_t  user_id,
                                const QString &auth,
                                int64_t        msg_id,
                                int            limit)
{
    QVector<Bus::MessageInfo> ret;
    if(!m_pChannel)
    {
        emit SignalGetMessageInfoResp(ErrorCode::ERR_SERVER_DISCONNECTED, ret);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::GetMessageInfoReq req;
    req.set_id(msg_id);
    req.set_session_id(session_id);
    req.set_limit(limit);
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());

    // Prepare the response and context
    GrpcLibrary::GetMessageInfoResp resp;
    grpc::ClientContext             context;

    // Make the RPC call
    grpc::Status status = stub->GetMessageInfo(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit SignalGetMessageInfoResp(ec, ret);
        return;
    }

    for(int i = 0; i < resp.messages_size(); i++)
    {
        const auto      &msg = resp.messages(i);
        Bus::MessageInfo item;
        _convert(item, msg);
        ret.append(item);
    }
    emit SignalGetMessageInfoResp(resp.error_code(), ret);
}

void GrpcClient::GetSession(const int64_t  id,
                            const int64_t  user_id,
                            const QString &auth,
                            int            limit)
{
    QVector<Bus::Session> ret;
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
        const auto  &sess = resp.sessions(i);
        Bus::Session item;
        _convert(item, sess);
        ret.append(item);
    }
    emit SignalGetSessionResp(resp.error_code(), ret);
}

void GrpcClient::NewSession(const int64_t  user_id,
                            const QString &auth,
                            const QString &title,
                            const QString &prompt,
                            const QString &model)
{
    Bus::Session ret;
    if(!m_pChannel)
    {
        emit SignalNewSessionResp(ErrorCode::ERR_SERVER_DISCONNECTED, ret);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::NewSessionReq req;
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    req.set_title(title.toStdString());
    req.set_content(prompt.toStdString());
    req.set_model(model.toStdString());

    // Prepare the response and context
    GrpcLibrary::NewSessionResp resp;
    grpc::ClientContext         context;

    // Make the RPC call
    grpc::Status status = stub->NewSession(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit SignalNewSessionResp(ec, ret);
        return;
    }

    _convert(ret, resp.session());
    emit SignalNewSessionResp(resp.error_code(), ret);
}

void GrpcClient::ModifySessionTitle(const int64_t  user_id,
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

void GrpcClient::DelSession(const int64_t           user_id,
                            const QString          &auth,
                            const QVector<int64_t> &ids)
{
    QVector<int64_t> ret;
    if(!m_pChannel)
    {
        emit SignalDelSessionResp(ErrorCode::ERR_SERVER_DISCONNECTED, ret);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::DelSessionReq req;
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    for(const auto &id : ids)
        req.add_ids(id);

    // Prepare the response and context
    GrpcLibrary::DelSessionResp resp;
    grpc::ClientContext         context;

    // Make the RPC call
    grpc::Status status = stub->DelSession(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit SignalDelSessionResp(ec, ret);
        return;
    }

    for(int i = 0; i < resp.ids_size(); i++)
        ret.append(resp.ids(i));

    emit SignalDelSessionResp(resp.error_code(), ret);
}

void GrpcClient::GetModelInfo(const int64_t  user_id,
                              const QString &auth,
                              const QString &hash,
                              int            limit)
{
    QVector<Bus::ModelConfig> ret;
    if(!m_pChannel)
    {
        emit SignalGetModelInfoResp(ErrorCode::ERR_SERVER_DISCONNECTED, ret);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::GetModelInfoReq req;
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    req.set_hash(hash.toStdString());
    req.set_limit(limit);

    // Prepare the response and context
    GrpcLibrary::GetModelInfoResp resp;
    grpc::ClientContext           context;

    // Make the RPC call
    grpc::Status status = stub->GetModelInfo(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit SignalGetModelInfoResp(ec, ret);
        return;
    }

    for(auto i = 0; i < resp.models_size(); i++)
    {
        const auto       item = resp.models(i);
        Bus::ModelConfig model;
        _convert(model, item);
        ret.append(model);
    }
    emit SignalGetModelInfoResp(resp.error_code(), ret);
}

void GrpcClient::NewModelInfo(const int64_t                    user_id,
                              const QString                   &auth,
                              const QVector<Bus::ModelConfig> &modelInfos)
{
    if(!m_pChannel)
    {
        emit SignalNewModelInfoResp(ErrorCode::ERR_SERVER_DISCONNECTED, {});
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::NewModelInfoReq req;
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    for(const auto &model : modelInfos)
    {
        auto m = req.add_models();
        _convert(*m, model);
    }

    // Prepare the response and context
    GrpcLibrary::NewModelInfoResp resp;
    grpc::ClientContext           context;

    // Make the RPC call
    grpc::Status status = stub->NewModelInfo(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit SignalNewModelInfoResp(ec, {});
        return;
    }

    QVector<QString> hashs;
    for(auto hash : resp.hashs())
        hashs.append(QString::fromStdString(hash));

    emit SignalNewModelInfoResp(resp.error_code(), hashs);
}

void GrpcClient::GetSkillInfo(const QString &hash, int limit)
{
    QVector<Bus::Skill> ret;
    if(!m_pChannel)
    {
        emit SignalGetSkillInfoResp(ErrorCode::ERR_SERVER_DISCONNECTED, ret);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::GetSkillInfoReq req;
    req.set_hash(hash.toStdString());
    req.set_limit(limit);

    // Prepare the response and context
    GrpcLibrary::GetSkillInfoResp resp;
    grpc::ClientContext           context;

    // Make the RPC call
    grpc::Status status = stub->GetSkillInfo(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit SignalGetSkillInfoResp(ec, ret);
        return;
    }

    for(int i = 0; i < resp.skills_size(); i++)
    {
        const auto &h = resp.skills(i);
        Bus::Skill  skill;
        _convert(skill, h);
        ret.append(skill);
    }
    emit SignalGetSkillInfoResp(resp.error_code(), ret);
}

void GrpcClient::Download(const QString &hash,
                          const int64_t  user_id,
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

void _convert(::GrpcLibrary::Session &dst, const Bus::Session &src)
{
    dst.set_id(src.id);
    dst.set_user_id(src.userId);
    dst.set_title(src.title.toStdString());
    dst.set_timestamp(src.timestamp.toStdString());
}

void GrpcClient::_convert(Bus::Session &dst, const ::GrpcLibrary::Session &src)
{
    dst.id        = src.id();
    dst.userId    = src.user_id();
    dst.title     = QString::fromStdString(src.title());
    dst.timestamp = QString::fromStdString(src.timestamp());
}

void GrpcClient::_convert(::GrpcLibrary::Model   &dst,
                          const Bus::ModelConfig &src)
{
    dst.set_hash(src.hash.toStdString());
    dst.set_name(src.name.toStdString());
    dst.set_publisher(src.publisher.toStdString());
    dst.set_timestamp(src.timestamp.toStdString());
    dst.set_addr(src.addr.toStdString());
    dst.set_capabilities(src.capabilities.toStdString());
    dst.set_context_size(src.contextSize);
    dst.set_cost(src.cost);
}

void GrpcClient::_convert(Bus::ModelConfig           &dst,
                          const ::GrpcLibrary::Model &src)
{
    dst.hash         = QString::fromStdString(src.hash());
    dst.name         = QString::fromStdString(src.name());
    dst.publisher    = QString::fromStdString(src.publisher());
    dst.timestamp    = QString::fromStdString(src.timestamp());
    dst.addr         = QString::fromStdString(src.addr());
    dst.capabilities = QString::fromStdString(src.capabilities());
    dst.contextSize  = src.context_size();
    dst.cost         = src.cost();
}

void GrpcClient::_convert(::GrpcLibrary::Skill &dst, const Bus::Skill &src)
{
    dst.set_hash(src.hash.toStdString());
    dst.set_name(src.name.toStdString());
    dst.set_desc(src.desc.toStdString());
    dst.set_publisher(src.publisher.toStdString());
    dst.set_version(src.version.toStdString());
    dst.set_timestamp(src.timestamp.toStdString());
    dst.set_platform(src.platform);
}

void GrpcClient::_convert(Bus::Skill &dst, const ::GrpcLibrary::Skill &src)
{
    dst.hash      = QString::fromStdString(src.hash());
    dst.name      = QString::fromStdString(src.name());
    dst.desc      = QString::fromStdString(src.desc());
    dst.publisher = QString::fromStdString(src.publisher());
    dst.version   = QString::fromStdString(src.version());
    dst.timestamp = QString::fromStdString(src.timestamp());
    dst.platform  = src.platform();
}

void GrpcClient::_convert(::GrpcLibrary::MessageInfo &dst,
                          const Bus::MessageInfo     &src)
{
    dst.set_id(src.id);
    dst.set_session_id(src.sessionId);
    dst.set_role(src.role.toStdString());
    dst.set_content(src.content.toStdString());
    dst.set_prev_message_id(src.prevMessageId);
    dst.set_timestamp(src.timestamp.toStdString());
}

void GrpcClient::_convert(Bus::MessageInfo                 &dst,
                          const ::GrpcLibrary::MessageInfo &src)
{
    dst.id            = src.id();
    dst.sessionId     = src.session_id();
    dst.role          = QString::fromStdString(src.role());
    dst.content       = QString::fromStdString(src.content());
    dst.prevMessageId = src.prev_message_id();
    dst.timestamp     = QString::fromStdString(src.timestamp());
}