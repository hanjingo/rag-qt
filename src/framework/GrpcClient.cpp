#include "GrpcClient.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>

#include "Error.h"
#include <libqt/sync/timedqueue.h>

#include "GrpcClientReactor.h"

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
    TimedQueue::Instance().start(10);
}

GrpcClient::~GrpcClient()
{
    if(m_pChannel)
    {
        delete m_pChannel;
        m_pChannel = nullptr;
    }
}

std::shared_ptr<RecognizeReactor>
GrpcClient::GetOrCreateRecognizeReactor(int64_t sessionId)
{
    if(!m_bIsConnected.load())
    {
        qWarning() << "Not connected to server, cannot create reactor";
        return nullptr;
    }

    auto it = m_recognizeReactors.find(sessionId);
    if(it != m_recognizeReactors.end())
        return it->second;

    auto reactor = std::make_shared<RecognizeReactor>(this, sessionId);
    m_recognizeReactors[sessionId] = reactor;

    // first call Recognize to start the reactor
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());
    stub->async()->Recognize(&reactor->m_context, reactor.get());
    reactor->StartCall();
    reactor->StartRead(&reactor->m_resp);
    return reactor;
}

void GrpcClient::RemoveRecognizeReactor(int64_t sessionId)
{
    m_recognizeReactors.erase(sessionId);
}

void GrpcClient::Connect(const QString &address)
{
    if(m_pChannel)
    {
        delete m_pChannel;
        m_pChannel = nullptr;
    }
    m_pChannel   = new hj::grpc_channel();
    m_strAddress = address;
    if(m_pChannel->connect(address.toStdString()))
    {
        Heartbeat(QDateTime::currentMSecsSinceEpoch());
    } else
    {
        m_bIsConnected.store(false);
        emit SignalGrpcConnectFailed(address);
    }
}

void GrpcClient::Heartbeat(const int64_t timestamp)
{
    if(!m_pChannel)
    {
        emit SignalGrpcConnectFailed("");
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::Ping req;
    req.set_timestamp(timestamp);

    // Prepare the response and context
    GrpcLibrary::Pong   resp;
    grpc::ClientContext context;

    // Make the RPC call
    grpc::Status status = stub->Heartbeat(&context, req, &resp);

    if(status.ok())
    {
        if(!m_bIsConnected.load())
        {
            m_bIsConnected.store(true);
            emit SignalGrpcConnected(m_strAddress);
        }
        emit SignalPong(timestamp);
    } else
    {
        m_bIsConnected.store(false);
        emit SignalGrpcDisconnected(m_strAddress);
    }
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

void GrpcClient::Query(const int64_t              id,
                       const int64_t              user_id,
                       const QString             &auth,
                       const QString             &content,
                       const QString             &model,
                       const Config::ModelConfig &config)
{
    if(!m_pChannel)
    {
        emit SignalQueryResp(ErrorCode::ERR_SERVER_DISCONNECTED, id, "", true);
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
    req.set_pipeline(config.pipeline.toStdString());
    req.set_api_key(config.apiKey.toStdString());

    // sampling parameters
    req.mutable_sampling()->set_penalty_last_n(config.penaltyLastN);
    req.mutable_sampling()->set_penalty_repeat(config.penaltyRepeat);
    req.mutable_sampling()->set_penalty_freq(config.penaltyFreq);
    req.mutable_sampling()->set_penalty_present(config.penaltyPresent);

    req.mutable_sampling()->set_temperature(config.temperature);
    req.mutable_sampling()->set_temperature_ext(config.temperatureExt);
    req.mutable_sampling()->set_temperature_ext_delta(
        config.temperatureExtDelta);
    req.mutable_sampling()->set_temperature_ext_exponent(
        config.temperatureExtExponent);

    req.mutable_sampling()->set_seed(config.seed);

    req.mutable_sampling()->set_top_k(config.topK);
    req.mutable_sampling()->set_top_p(config.topP);
    req.mutable_sampling()->set_top_p_min_keep(config.topPMinKeep);
    req.mutable_sampling()->set_min_p(config.minP);
    req.mutable_sampling()->set_min_p_min_keep(config.minPMinKeep);

    // context params
    req.mutable_ctx()->set_window_size(config.ctxWindowSize);
    req.mutable_ctx()->set_stop_words(config.stopWords.toStdString());

    req.mutable_ctx()->set_n_ctx(config.nCtx);
    req.mutable_ctx()->set_n_batch(config.nBatch);
    req.mutable_ctx()->set_n_ubatch(config.nUbatch);
    req.mutable_ctx()->set_n_seq_max(config.nSeqMax);
    req.mutable_ctx()->set_n_threads(config.nThreads);
    req.mutable_ctx()->set_n_threads_batch(config.nThreadsBatch);

    req.mutable_ctx()->set_rope_freq_base(config.ropeFreqBase);
    req.mutable_ctx()->set_rope_freq_scale(config.ropeFreqScale);
    req.mutable_ctx()->set_yarn_ext_factor(config.yarnExtFactor);
    req.mutable_ctx()->set_yarn_attn_factor(config.yarnAttnFactor);
    req.mutable_ctx()->set_yarn_beta_fast(config.yarnBetaFast);
    req.mutable_ctx()->set_yarn_beta_slow(config.yarnBetaSlow);
    req.mutable_ctx()->set_yarn_orig_ctx(config.yarnOrigCtx);
    req.mutable_ctx()->set_defrag_thold(config.defragThold);

    req.mutable_ctx()->set_embeddings(config.embeddings);
    req.mutable_ctx()->set_offload_kqv(config.offloadKQV);
    req.mutable_ctx()->set_no_perf(config.noPerf);
    req.mutable_ctx()->set_op_offload(config.opOffload);
    req.mutable_ctx()->set_swa_full(config.swaFull);
    req.mutable_ctx()->set_kv_unified(config.kvUnified);

    req.mutable_ctx()->set_prompt(config.prompt.toStdString());

    // Prepare the response and context
    GrpcLibrary::QueryResp resp;
    grpc::ClientContext    context;

    QueryReactor *reactor = new QueryReactor(this, id);
    stub->async()->Query(&reactor->m_context, &req, reactor);
    reactor->StartCall();
    reactor->StartRead(&reactor->m_resp);
}

void GrpcClient::StopAnswer(const int64_t  session_id,
                            const int64_t  user_id,
                            const QString &auth)
{
    if(!m_pChannel)
    {
        emit SignalStopAnswerResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                  session_id);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::StopAnswerReq req;
    req.set_session_id(session_id);
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());

    // Prepare the response and context
    GrpcLibrary::StopAnswerResp resp;
    grpc::ClientContext         context;

    // Make the RPC call
    grpc::Status status = stub->StopAnswer(&context, req, &resp);

    if(status.ok())
        emit SignalStopAnswerResp(resp.error_code(), session_id);
    else
        emit SignalStopAnswerResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                  session_id);
}

void GrpcClient::Recognize(const int64_t                  session_id,
                           const int64_t                  user_id,
                           const QString                 &auth,
                           const QByteArray              &data,
                           const Config::TranslatorParam &params,
                           const QString                 &translatorId)
{
    if(!m_pChannel)
    {
        emit SignalRecognizeResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                 QString("Server disconnected"),
                                 true,
                                 0.0);
        return;
    }

    auto reactor = GetOrCreateRecognizeReactor(session_id);
    if(!reactor)
    {
        emit SignalRecognizeResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                 QString("Server not connected"),
                                 true,
                                 0.0);
        return;
    }

    GrpcLibrary::RecognitionParam param;
    param.set_n_threads(params.nThreads);
    param.set_n_max_text_ctx(params.nMaxTextCtx);
    param.set_offset_ms(params.offsetMs);
    param.set_duration_ms(params.durationMs);
    param.set_translate(params.translate);
    param.set_detect_language(params.detectLanguage);
    param.set_language(params.language.toStdString());
    param.set_no_ctx(params.noCtx);
    param.set_no_timestamps(params.noTimestamps);
    param.set_single_segment(params.singleSegment);
    param.set_print_special(params.printSpecial);
    param.set_print_progress(params.printProgress);
    param.set_print_realtime(params.printRealtime);
    param.set_print_timestamps(params.printTimestamps);
    param.set_carry_initial_prompt(params.carryInitialPrompt);
    param.set_initial_prompt(params.initialPrompt.toStdString());
    param.set_suppress_regex(params.suppressRegex.toStdString());
    param.set_suppress_blank(params.suppressBlank);
    param.set_suppress_nst(params.suppressNst);
    param.set_temperature(params.temperature);
    param.set_temperature_inc(params.temperatureInc);
    param.set_max_initial_ts(params.maxInitialTs);
    param.set_length_penalty(params.lengthPenalty);
    param.set_entropy_thold(params.entropyThold);
    param.set_logprob_thold(params.logprobThold);
    param.set_no_speech_thold(params.noSpeechThold);

    // send config
    GrpcLibrary::RecognizeReq configReq;
    configReq.set_ctx_id(translatorId.toStdString());
    configReq.set_session_id(session_id);
    configReq.mutable_param()->CopyFrom(param);
    reactor->SendRequest(configReq);

    // send audio data
    if(!data.isEmpty())
    {
        GrpcLibrary::RecognizeReq audioReq;
        audioReq.set_ctx_id(translatorId.toStdString());
        audioReq.set_session_id(session_id);
        audioReq.set_audio_chunk(data.data(), data.size());
        reactor->SendRequest(audioReq);
    }

    qDebug() << "Starting recognition with param:";
}

void GrpcClient::RecognizeStop(const int64_t  session_id,
                               const int64_t  user_id,
                               const QString &auth)
{
    if(!m_pChannel)
    {
        emit SignalStopRecognizeResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                     session_id);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::StopRecognizeReq req;
    req.set_session_id(session_id);
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());

    // Prepare the response and context
    GrpcLibrary::StopRecognizeResp resp;
    grpc::ClientContext            context;

    // Make the RPC call
    grpc::Status status = stub->StopRecognize(&context, req, &resp);

    if(status.ok())
        emit SignalStopRecognizeResp(resp.error_code(), session_id);
    else
        emit SignalStopRecognizeResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                     session_id);
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

void GrpcClient::Upload(const QString &hash,
                        const int64_t  user_id,
                        const QString &auth,
                        const QString &addr,
                        const int64_t  size_kb)
{
    if(!m_pChannel)
    {
        emit SignalUploadResp(ErrorCode::ERR_SERVER_DISCONNECTED, addr);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibrary::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibrary::UploadReq req;
    req.set_hash(hash.toStdString());
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    req.set_addr(addr.toStdString());
    req.set_size_kb(size_kb);

    // Prepare the response and context
    GrpcLibrary::UploadResp resp;
    grpc::ClientContext     context;

    // Make the RPC call
    grpc::Status status = stub->Upload(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit SignalUploadResp(ec, addr);
        return;
    }

    emit SignalUploadResp(resp.error_code(), addr);
}

void GrpcClient::OnConnectionLost()
{
    m_bIsConnected.store(false);
    emit SignalGrpcDisconnected(m_strAddress);
    for(auto &pair : m_recognizeReactors)
    {
        if(pair.second)
            pair.second->OnConnectionLost();
    }
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