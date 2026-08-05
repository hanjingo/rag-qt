#include "GrpcClient.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>

#include "Error.h"
#include <libqt/sync/timedqueue.h>

#include "GrpcClientReactor.h"
#include "System.h"

static std::unordered_map<int64_t, std::shared_ptr<RecognizeReactor>>
    m_recognizeReactors;
static std::unordered_map<int64_t, std::shared_ptr<EmbeddingReactor>>
    m_embeddingRectors;

GrpcClient::GrpcClient(QObject *parent)
    : QObject(parent)
    , m_pChannel(nullptr)
{
    TimedQueue::instance().start(10);
    qDebug() << "GrpcClient created";
}

GrpcClient::~GrpcClient()
{
    if(m_pChannel)
    {
        disconnect();
        delete m_pChannel;
        m_pChannel = nullptr;
    }
}

void GrpcClient::RemoveRecognizeReactor(int64_t sessionId)
{
    m_recognizeReactors.erase(sessionId);
}

void GrpcClient::RemoveEmbeddingReactor(int64_t taskId)
{
    m_embeddingRectors.erase(taskId);
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
        emit signalGrpcConnectFailed(address);
    }
}

void GrpcClient::Heartbeat(const int64_t timestamp)
{
    if(!m_pChannel)
    {
        emit signalGrpcConnectFailed("");
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::Ping req;
    req.set_timestamp(timestamp);

    // Prepare the response and context
    GrpcLibraryV1::Pong resp;
    grpc::ClientContext context;

    // Make the RPC call
    grpc::Status status = stub->Heartbeat(&context, req, &resp);

    if(status.ok())
    {
        if(!m_bIsConnected.load())
        {
            m_bIsConnected.store(true);
            emit signalGrpcConnected(m_strAddress);
        }
        emit signalPong(timestamp);
    } else
    {
        m_bIsConnected.store(false);
        emit signalGrpcDisconnected(m_strAddress);
    }
}

void GrpcClient::Login(const QString &username, const QString &password)
{
    if(!m_pChannel)
    {
        emit signalLoginResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                             -1,
                             "",
                             username,
                             "",
                             false);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::LoginReq req;
    req.set_account(username.toStdString());
    req.set_passwd(password.toStdString());
    req.set_platform(System::instance()->Platform().toStdString());
    req.set_arch(System::instance()->Arch().toStdString());
    req.set_client_version(System::instance()->Version().toStdString());

    // Prepare the response and context
    GrpcLibraryV1::LoginResp resp;
    grpc::ClientContext      context;

    // Make the RPC call
    grpc::Status status = stub->Login(&context, req, &resp);

    bool is_force_update = resp.update_info().force_update();
    if(status.ok())
        emit signalLoginResp(resp.error_code(),
                             resp.user_id(),
                             QString::fromStdString(resp.auth()),
                             QString::fromStdString(resp.account()),
                             QString::fromStdString(resp.last_login_time()),
                             is_force_update);
    else
        emit signalLoginResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                             -1,
                             "",
                             username,
                             "",
                             false);
}

void GrpcClient::Logout(const int64_t user_id, const QString &auth)
{
    if(!m_pChannel)
    {
        emit signalLogoutResp(ErrorCode::ERR_SERVER_DISCONNECTED, -1);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::LogoutReq req;
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());

    // Prepare the response and context
    GrpcLibraryV1::LogoutResp resp;
    grpc::ClientContext       context;

    // Make the RPC call
    grpc::Status status = stub->Logout(&context, req, &resp);

    if(status.ok())
        emit signalLogoutResp(resp.error_code(), resp.user_id());
    else
        emit signalLogoutResp(ErrorCode::ERR_SERVER_DISCONNECTED, -1);
}

void GrpcClient::RegAccount(const QString &username, const QString &password)
{
    if(!m_pChannel)
    {
        emit signalRegAccountResp(ErrorCode::ERR_SERVER_DISCONNECTED, -1);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::RegAccountReq req;
    req.set_account(username.toStdString());
    req.set_passwd(password.toStdString());

    // Prepare the response and context
    GrpcLibraryV1::RegAccountResp resp;
    grpc::ClientContext           context;

    // Make the RPC call
    grpc::Status status = stub->RegAccount(&context, req, &resp);

    if(status.ok())
        emit signalRegAccountResp(resp.error_code(), resp.user_id());
    else
        emit signalRegAccountResp(ErrorCode::ERR_SERVER_DISCONNECTED, -1);
}

void GrpcClient::Query(const int64_t              id,
                       const int64_t              msgId,
                       const int64_t              userId,
                       const QString             &auth,
                       const QString             &content,
                       const QString             &model,
                       const Config::ModelConfig &config)
{
    if(!m_pChannel)
    {
        emit signalQueryResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                             id,
                             -1,
                             "",
                             true);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::QueryReq req;
    req.set_id(id);
    req.set_msg_id(msgId);
    req.set_user_id(userId);
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
    GrpcLibraryV1::QueryResp resp;
    grpc::ClientContext      context;

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
        emit signalStopAnswerResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                  session_id);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::StopAnswerReq req;
    req.set_session_id(session_id);
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());

    // Prepare the response and context
    GrpcLibraryV1::StopAnswerResp resp;
    grpc::ClientContext           context;

    // Make the RPC call
    grpc::Status status = stub->StopAnswer(&context, req, &resp);

    if(status.ok())
        emit signalStopAnswerResp(resp.error_code(), session_id);
    else
        emit signalStopAnswerResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                  session_id);
}

void GrpcClient::Recognize(const int64_t           session_id,
                           const int64_t           user_id,
                           const QString          &auth,
                           const QByteArray       &data,
                           const Config::AsrParam &params,
                           const QString          &translatorId)
{
    if(!m_pChannel || !m_bIsConnected.load())
    {
        emit signalRecognizeResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                 QString("Server disconnected"),
                                 true,
                                 0.0);
        return;
    }

    // get or make reactor
    std::shared_ptr<RecognizeReactor> reactor;
    auto                              it = m_recognizeReactors.find(session_id);
    if(it == m_recognizeReactors.end())
    {
        reactor = std::make_shared<RecognizeReactor>(this, session_id);
        m_recognizeReactors[session_id] = reactor;

        // first call Recognize to start the reactor
        auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());
        stub->async()->Recognize(&reactor->m_context, reactor.get());
        reactor->StartCall();
        reactor->StartRead(&reactor->m_resp);

        // first call Recognize to send the config
        GrpcLibraryV1::RecognitionParam param;
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
        param.set_vad(params.vad);
        param.set_vad_model_path(params.vadModelPath.toStdString());
        param.mutable_vad_params()->set_threshold(params.vadParams.threshold);
        param.mutable_vad_params()->set_min_speech_dur_ms(
            params.vadParams.minSpeechDurMs);
        param.mutable_vad_params()->set_min_silence_dur_ms(
            params.vadParams.minSilenceDurMs);
        param.mutable_vad_params()->set_max_speech_dur_s(
            params.vadParams.maxSpeechDurS);
        param.mutable_vad_params()->set_speech_pad_ms(
            params.vadParams.speechPadMs);
        param.mutable_vad_params()->set_samples_overlap(
            params.vadParams.samplesOverlap);
        qDebug() << "Recognize called with params(session_id:" << session_id
                 << ", user_id:" << user_id << ", translatorId:" << translatorId
                 << ", data.size():" << data.size()
                 << ", params: nThreads=" << params.nThreads
                 << ", nMaxTextCtx=" << params.nMaxTextCtx
                 << ", offsetMs=" << params.offsetMs
                 << ", durationMs=" << params.durationMs
                 << ", translate=" << params.translate
                 << ", detectLanguage=" << params.detectLanguage
                 << ", language=" << params.language
                 << ", noCtx=" << params.noCtx
                 << ", noTimestamps=" << params.noTimestamps
                 << ", singleSegment=" << params.singleSegment
                 << ", printSpecial=" << params.printSpecial
                 << ", printProgress=" << params.printProgress
                 << ", printRealtime=" << params.printRealtime
                 << ", printTimestamps=" << params.printTimestamps
                 << ", carryInitialPrompt=" << params.carryInitialPrompt
                 << ", initialPrompt=" << params.initialPrompt
                 << ", suppressRegex=" << params.suppressRegex
                 << ", suppressBlank=" << params.suppressBlank
                 << ", suppressNst=" << params.suppressNst
                 << ", temperature=" << params.temperature
                 << ", temperatureInc=" << params.temperatureInc
                 << ", maxInitialTs=" << params.maxInitialTs
                 << ", lengthPenalty=" << params.lengthPenalty
                 << ", entropyThold=" << params.entropyThold
                 << ", logprobThold=" << params.logprobThold
                 << ", noSpeechThold=" << params.noSpeechThold
                 << ", vad=" << params.vad
                 << ", vadModelPath=" << params.vadModelPath
                 << ", vadParams.threshold=" << params.vadParams.threshold
                 << ", vadParams.minSpeechDurMs="
                 << params.vadParams.minSpeechDurMs
                 << ", vadParams.minSilenceDurMs="
                 << params.vadParams.minSilenceDurMs
                 << ", vadParams.maxSpeechDurS="
                 << params.vadParams.maxSpeechDurS
                 << ", vadParams.speechPadMs=" << params.vadParams.speechPadMs
                 << ", vadParams.samplesOverlap="
                 << params.vadParams.samplesOverlap << ")";

        // send config
        GrpcLibraryV1::RecognizeReq configReq;
        configReq.set_ctx_id(translatorId.toStdString());
        configReq.set_session_id(session_id);
        configReq.mutable_param()->CopyFrom(param);
        reactor->SendRequest(configReq);
    } else
    {
        reactor = m_recognizeReactors[session_id];
    }

    // send audio data
    if(!data.isEmpty())
    {
        GrpcLibraryV1::RecognizeReq audioReq;
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
        emit signalStopRecognizeResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                     session_id);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::StopRecognizeReq req;
    req.set_session_id(session_id);
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());

    // Prepare the response and context
    GrpcLibraryV1::StopRecognizeResp resp;
    grpc::ClientContext              context;

    // Make the RPC call
    grpc::Status status = stub->StopRecognize(&context, req, &resp);

    if(status.ok())
        emit signalStopRecognizeResp(resp.error_code(), session_id);
    else
        emit signalStopRecognizeResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                     session_id);
}

void GrpcClient::Embedding(const int64_t               task_id,
                           const int64_t               user_id,
                           const QString              &auth,
                           const int64_t               chunk_id,
                           const QByteArray           &chunk_data,
                           const int64_t               start_pos,
                           const int64_t               end_pos,
                           const Config::MemoryConfig &params)
{
    qDebug() << "Embedding entry";
    if(!m_pChannel || !m_bIsConnected.load())
    {
        emit signalEmbeddingResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                 task_id,
                                 0,
                                 "");
        return;
    }

    std::shared_ptr<EmbeddingReactor> reactor;
    auto                              it = m_embeddingRectors.find(task_id);
    if(it == m_embeddingRectors.end())
    {
        reactor = std::make_shared<EmbeddingReactor>(this, task_id);
        m_embeddingRectors[task_id] = reactor;

        // first call Embedding to start the reactor
        auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());
        stub->async()->Embedding(&reactor->m_context, reactor.get());
        reactor->StartCall();
        reactor->StartRead(&reactor->m_resp);
        qDebug() << "create new EmbeddingReactor for task_id:" << task_id;
    } else
    {
        reactor = m_embeddingRectors[task_id];
    }

    if(!params.id.isEmpty())
    {
        GrpcLibraryV1::EmbeddingParam param;
        param.set_dimension(params.dimension);

        GrpcLibraryV1::EmbeddingReq req;
        req.set_task_id(task_id);
        req.mutable_param()->CopyFrom(param);
        reactor->SendRequest(req);
        qDebug() << "Starting embedding with param: dimension="
                 << params.dimension;
    }

    if(!chunk_data.isEmpty())
    {
        GrpcLibraryV1::FileChunk chunk;
        chunk.set_id(chunk_id);
        chunk.set_start_pos(start_pos);
        chunk.set_end_pos(end_pos);
        chunk.set_filename(params.originFilePath.toStdString());
        chunk.set_data(chunk_data.data(), chunk_data.size());

        GrpcLibraryV1::EmbeddingReq req;
        req.set_task_id(task_id);
        req.mutable_chunk()->CopyFrom(chunk);
        reactor->SendRequest(req);
        qDebug() << "Starting embedding with chunk: id=" << chunk_id
                 << ",start_pos=" << start_pos << ",end_pos=" << end_pos
                 << ",data.size()=" << chunk_data.size();
    }
}

void GrpcClient::EmbeddingStop(const int64_t  task_id,
                               const int64_t  user_id,
                               const QString &auth)
{
    if(!m_pChannel)
    {
        emit signalStopEmbeddingResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                     task_id);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::StopEmbeddingReq req;
    req.set_task_id(task_id);
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());

    // Prepare the response and context
    GrpcLibraryV1::StopEmbeddingResp resp;
    grpc::ClientContext              context;

    // Make the RPC call
    grpc::Status status = stub->StopEmbedding(&context, req, &resp);

    if(status.ok())
        emit signalStopEmbeddingResp(resp.error_code(), task_id);
    else
        emit signalStopEmbeddingResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                     task_id);
}

void GrpcClient::GetChatMessage(const int64_t  session_id,
                                const int64_t  user_id,
                                const QString &auth,
                                int64_t        msg_id,
                                int            limit)
{
    QVector<Bus::ChatMessage> ret;
    if(!m_pChannel)
    {
        emit signalGetChatMessageResp(ErrorCode::ERR_SERVER_DISCONNECTED, ret);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::GetChatMessageReq req;
    req.set_id(msg_id);
    req.set_session_id(session_id);
    req.set_limit(limit);
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());

    // Prepare the response and context
    GrpcLibraryV1::GetChatMessageResp resp;
    grpc::ClientContext               context;

    // Make the RPC call
    grpc::Status status = stub->GetChatMessage(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit signalGetChatMessageResp(ec, ret);
        return;
    }

    for(int i = 0; i < resp.messages_size(); i++)
    {
        const auto      &msg = resp.messages(i);
        Bus::ChatMessage item;
        _convert(item, msg);
        ret.append(item);
    }
    emit signalGetChatMessageResp(resp.error_code(), ret);
}

void GrpcClient::GetSession(const int64_t  id,
                            const int64_t  user_id,
                            const QString &auth,
                            int            limit)
{
    QVector<Bus::Session> ret;
    if(!m_pChannel)
    {
        emit signalGetSessionResp(ErrorCode::ERR_SERVER_DISCONNECTED, ret);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::GetSessionReq req;
    req.set_id(id);
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    req.set_limit(limit);

    // Prepare the response and context
    GrpcLibraryV1::GetSessionResp resp;
    grpc::ClientContext           context;

    // Make the RPC call
    grpc::Status status = stub->GetSession(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit signalGetSessionResp(ec, ret);
        return;
    }

    for(int i = 0; i < resp.sessions_size(); i++)
    {
        const auto  &sess = resp.sessions(i);
        Bus::Session item;
        _convert(item, sess);
        ret.append(item);
    }
    emit signalGetSessionResp(resp.error_code(), ret);
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
        emit signalNewSessionResp(ErrorCode::ERR_SERVER_DISCONNECTED, ret);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::NewSessionReq req;
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    req.set_title(title.toStdString());
    req.set_content(prompt.toStdString());
    req.set_model(model.toStdString());

    // Prepare the response and context
    GrpcLibraryV1::NewSessionResp resp;
    grpc::ClientContext           context;

    // Make the RPC call
    grpc::Status status = stub->NewSession(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit signalNewSessionResp(ec, ret);
        return;
    }

    _convert(ret, resp.session());
    emit signalNewSessionResp(resp.error_code(), ret);
}

void GrpcClient::ModifySessionTitle(const int64_t  user_id,
                                    const QString &auth,
                                    const int64_t  id,
                                    const QString &title)
{
    if(!m_pChannel)
    {
        emit signalModifySessionTitleResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                          id,
                                          title);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::ModifySessionTitleReq req;
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    req.set_id(id);
    req.set_title(title.toStdString());

    // Prepare the response and context
    GrpcLibraryV1::ModifySessionTitleResp resp;
    grpc::ClientContext                   context;

    // Make the RPC call
    grpc::Status status = stub->ModifySessionTitle(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit signalModifySessionTitleResp(ec, id, title);
        return;
    }

    emit signalModifySessionTitleResp(resp.error_code(), id, title);
}

void GrpcClient::DelSession(const int64_t           user_id,
                            const QString          &auth,
                            const QVector<int64_t> &ids)
{
    QVector<int64_t> ret;
    if(!m_pChannel)
    {
        emit signalDelSessionResp(ErrorCode::ERR_SERVER_DISCONNECTED, ret);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::DelSessionReq req;
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    for(const auto &id : ids)
        req.add_ids(id);

    // Prepare the response and context
    GrpcLibraryV1::DelSessionResp resp;
    grpc::ClientContext           context;

    // Make the RPC call
    grpc::Status status = stub->DelSession(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit signalDelSessionResp(ec, ret);
        return;
    }

    for(int i = 0; i < resp.ids_size(); i++)
        ret.append(resp.ids(i));

    emit signalDelSessionResp(resp.error_code(), ret);
}

void GrpcClient::GetPluginInfo(const QString &hash,
                               const QString &publisher,
                               int            limit)
{
    QVector<Bus::Plugin> ret;
    if(!m_pChannel)
    {
        emit signalGetPluginInfoResp(ErrorCode::ERR_SERVER_DISCONNECTED, ret);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::GetPluginInfoReq req;
    req.set_hash(hash.toStdString());
    req.set_publisher(publisher.toStdString());
    req.set_limit(limit);

    // Prepare the response and context
    GrpcLibraryV1::GetPluginInfoResp resp;
    grpc::ClientContext              context;

    // Make the RPC call
    grpc::Status status = stub->GetPluginInfo(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit signalGetPluginInfoResp(ec, ret);
        return;
    }

    for(int i = 0; i < resp.plugins_size(); i++)
    {
        const auto &h = resp.plugins(i);
        Bus::Plugin plugin;
        _convert(plugin, h);
        ret.append(plugin);
    }
    emit signalGetPluginInfoResp(resp.error_code(), ret);
}

void GrpcClient::Download(const QString &hash,
                          const int64_t  user_id,
                          const QString &auth)
{
    if(!m_pChannel)
    {
        emit signalDownloadResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                hash,
                                "",
                                0);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::DownloadReq req;
    req.set_hash(hash.toStdString());
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());

    // Prepare the response and context
    GrpcLibraryV1::DownloadResp resp;
    grpc::ClientContext         context;

    // Make the RPC call
    grpc::Status status = stub->Download(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit signalDownloadResp(ec, hash, "", 0);
        return;
    }

    QString addr = QString::fromStdString(resp.addr());
    emit    signalDownloadResp(resp.error_code(), hash, addr, resp.size_kb());
}

void GrpcClient::Upload(const QString &hash,
                        const int64_t  user_id,
                        const QString &auth,
                        const QString &addr,
                        const int64_t  size_kb)
{
    if(!m_pChannel)
    {
        emit signalUploadResp(ErrorCode::ERR_SERVER_DISCONNECTED, addr);
        return;
    }

    // Create a stub for the gRPC service
    auto stub = GrpcLibraryV1::GrpcService::NewStub(m_pChannel->get());

    // Prepare the request
    GrpcLibraryV1::UploadReq req;
    req.set_hash(hash.toStdString());
    req.set_user_id(user_id);
    req.set_auth(auth.toStdString());
    req.set_addr(addr.toStdString());
    req.set_size_kb(size_kb);

    // Prepare the response and context
    GrpcLibraryV1::UploadResp resp;
    grpc::ClientContext       context;

    // Make the RPC call
    grpc::Status status = stub->Upload(&context, req, &resp);

    if(!status.ok())
    {
        auto ec = status.error_code();
        emit signalUploadResp(ec, addr);
        return;
    }

    emit signalUploadResp(resp.error_code(), addr);
}

void GrpcClient::OnConnectionLost()
{
    m_bIsConnected.store(false);
    emit signalGrpcDisconnected(m_strAddress);
    for(auto &pair : m_recognizeReactors)
    {
        if(pair.second)
            pair.second->OnConnectionLost();
    }
}

void _convert(::GrpcLibraryV1::Session &dst, const Bus::Session &src)
{
    dst.set_id(src.id);
    dst.set_user_id(src.userId);
    dst.set_title(src.title.toStdString());
    dst.set_timestamp(src.timestamp.toStdString());
}

void GrpcClient::_convert(Bus::Session                   &dst,
                          const ::GrpcLibraryV1::Session &src)
{
    dst.id        = src.id();
    dst.userId    = src.user_id();
    dst.title     = QString::fromStdString(src.title());
    dst.timestamp = QString::fromStdString(src.timestamp());
}

void GrpcClient::_convert(::GrpcLibraryV1::Plugin &dst, const Bus::Plugin &src)
{
    dst.set_hash(src.hash.toStdString());
    dst.set_name(src.name.toStdString());
    dst.set_desc(src.desc.toStdString());
    dst.set_publisher(src.publisher.toStdString());
    dst.set_version(src.version.toStdString());
    dst.set_timestamp(src.timestamp.toStdString());
    dst.set_platform(src.platform);
}

void GrpcClient::_convert(Bus::Plugin &dst, const ::GrpcLibraryV1::Plugin &src)
{
    dst.hash      = QString::fromStdString(src.hash());
    dst.name      = QString::fromStdString(src.name());
    dst.desc      = QString::fromStdString(src.desc());
    dst.publisher = QString::fromStdString(src.publisher());
    dst.version   = QString::fromStdString(src.version());
    dst.timestamp = QString::fromStdString(src.timestamp());
    dst.platform  = src.platform();
}

void GrpcClient::_convert(::GrpcLibraryV1::ChatMessage &dst,
                          const Bus::ChatMessage       &src)
{
    dst.set_id(src.id);
    dst.set_session_id(src.sessionId);
    dst.set_role(src.role.toStdString());
    dst.set_content(src.content.toStdString());
    dst.set_prev_message_id(src.prevMessageId);
    dst.set_timestamp(src.timestamp.toStdString());
}

void GrpcClient::_convert(Bus::ChatMessage                   &dst,
                          const ::GrpcLibraryV1::ChatMessage &src)
{
    dst.id            = src.id();
    dst.sessionId     = src.session_id();
    dst.role          = QString::fromStdString(src.role());
    dst.content       = QString::fromStdString(src.content());
    dst.prevMessageId = src.prev_message_id();
    dst.timestamp     = QString::fromStdString(src.timestamp());
}