#include "GrpcClientReactor.h"

#include <QDebug>
#include <QCoreApplication>

#include "Error.h"
#include <libqt/sync/timedqueue.h>

void QueryReactor::OnReadDone(bool ok)
{
    if(ok)
    {
        auto ec         = m_resp.error_code();
        auto id         = m_id;
        auto content    = QString::fromStdString(m_resp.content());
        auto isFinished = m_resp.is_finished();
        auto msgId      = m_resp.msg_id();
        qDebug() << "Async Received Query response, error_code:" << ec
                 << ", session id:" << id << ", msg id:" << msgId
                 << ", content:" << content << ", is_finished:" << isFinished;

        TimedQueue::instance().enqueue([ec, id, msgId, content, isFinished]() {
            emit GrpcClient::instance()
                -> signalQueryResp(ec, id, msgId, content, isFinished);
        });

        StartRead(&m_resp);
    }
}

void QueryReactor::OnDone(const grpc::Status &status)
{
    qDebug() << "Async Query stream finished for id:" << m_id << " Status ok? "
             << status.ok();

    if(!status.ok())
    {
        emit m_client->signalQueryResp(
            ErrorCode::ERR_SERVER_DISCONNECTED,
            m_id,
            -1,
            QString::fromStdString(status.error_message()),
            true);
    }

    delete this;
}

// ----------------------------------- RecognizeAudioReactor ----------------------------
RecognizeAudioReactor::RecognizeAudioReactor(QPointer<GrpcClient> client,
                                             int64_t              sessionId)
    : m_client(client)
    , m_sessionId(sessionId)
    , m_writeCh(10)
    , m_isWriting(false)
    , m_isDone(false)
{
    qDebug() << "RecognizeAudioReactor created for session_id:" << sessionId;
}

RecognizeAudioReactor::~RecognizeAudioReactor()
{
    qDebug() << "RecognizeAudioReactor destroyed for session_id:"
             << m_sessionId;
}

void RecognizeAudioReactor::SendRequest(
    const GrpcLibraryV1::RecognizeAudioReq &req)
{
    if(m_isDone.load())
    {
        qWarning() << "Cannot send request, stream is done for session_id:"
                   << req.session_id();
        emit m_client->signalRecognizeAudioResp(
            ErrorCode::ERR_SERVER_DISCONNECTED,
            "Server disconnected",
            true,
            0.0);
        return;
    }

    m_writeCh.enqueue(req);
    _flush();
}

void RecognizeAudioReactor::_flush()
{
    if(m_isDone.load())
        return;

    if(m_isWriting.load())
        return;

    GrpcLibraryV1::RecognizeAudioReq req;
    if(!m_writeCh.try_dequeue(req))
        return;

    m_isWriting.store(true);
    qDebug() << "Flushing request for session_id:" << req.session_id()
             << ", ctx_id:" << req.ctx_id().c_str()
             << ", has_param:" << req.has_param()
             << ", has_audio:" << req.has_audio_chunk();
    StartWrite(&req);
}

void RecognizeAudioReactor::_pull()
{
    if(m_isDone.load())
        return;

    auto ec         = m_resp.error_code();
    auto transcript = QString::fromStdString(m_resp.transcript());
    auto isFinished = m_resp.is_finished();
    auto confidence = m_resp.confidence();

    qDebug() << "Received recognition result for session_id:" << m_sessionId
             << "error_code=" << ec << "transcript=" << transcript
             << "is_finished=" << isFinished << "confidence=" << confidence;

    TimedQueue::instance().enqueue([ec, transcript, isFinished, confidence]() {
        emit GrpcClient::instance()
            -> signalRecognizeAudioResp(ec, transcript, isFinished, confidence);
    });

    if(!isFinished && !m_isDone.load())
    {
        StartRead(&m_resp);
    } else if(isFinished)
    {
        m_isDone.store(true);
    }
}

void RecognizeAudioReactor::OnWriteDone(bool ok)
{
    m_isWriting.store(false);

    if(!ok)
    {
        qWarning() << "Write failed for session_id:" << m_sessionId
                   << ", cancelling";
        m_isDone.store(true);

        emit m_client->signalRecognizeAudioResp(
            ErrorCode::ERR_SERVER_DISCONNECTED,
            "Write failed - connection lost",
            true,
            0.0);
        m_client->RemoveRecognizeReactor(GetSessionId());
        return;
    }

    _flush();
}

void RecognizeAudioReactor::OnReadDone(bool ok)
{
    if(!ok)
    {
        qDebug() << "Read completed (no more responses) for session_id:"
                 << m_sessionId;
        m_isDone.store(true);
        return;
    }

    _pull();
}

void RecognizeAudioReactor::OnDone(const grpc::Status &status)
{
    qDebug() << "RecognizeAudio stream finished for session_id:" << m_sessionId
             << "status ok?" << status.ok()
             << "error:" << status.error_message().c_str();

    if(!status.ok())
    {
        QString errorMsg = QString::fromStdString(status.error_message());
        emit    m_client->signalRecognizeAudioResp(
            ErrorCode::ERR_SERVER_DISCONNECTED,
            errorMsg,
            true,
            0.0);
    }

    m_isDone.store(true);
    m_isWriting.store(false);
    m_client->RemoveRecognizeReactor(GetSessionId());
}

void RecognizeAudioReactor::OnConnectionLost()
{
    m_isDone.store(true);
    m_isWriting.store(false);
    GrpcLibraryV1::RecognizeAudioReq req;
    while(m_writeCh.try_dequeue(req))
    {
    }
}

// ----------------------------------- EmbeddingReactor ----------------------------
EmbeddingReactor::EmbeddingReactor(QPointer<GrpcClient> client, int64_t taskId)
    : m_client(client)
    , m_taskId(taskId)
    , m_writeCh(10)
    , m_isWriting(false)
    , m_isReading(false)
    , m_isDone(false)
{
    qDebug() << "EmbeddingReactor created for task_id:" << taskId;
}

EmbeddingReactor::~EmbeddingReactor()
{
    qDebug() << "EmbeddingReactor destroyed for task_id:" << m_taskId;
}

void EmbeddingReactor::SendRequest(const GrpcLibraryV1::EmbeddingReq &req)
{
    if(!m_client || !m_client->IsConnected())
    {
        qWarning() << "Client disconnected, cannot send request for task_id:"
                   << m_taskId;
        emit m_client->signalEmbeddingResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                           m_taskId,
                                           0,
                                           "");
        return;
    }

    if(m_isDone.load())
        return;

    qDebug() << "send request for task_id:" << req.task_id()
             << ", has_chunk:" << req.has_chunk()
             << ", has_param:" << req.has_param();
    m_writeCh.enqueue(req);
    _flush();
}

void EmbeddingReactor::_flush()
{
    if(m_isDone.load())
        return;

    if(m_isWriting.load())
        return;

    GrpcLibraryV1::EmbeddingReq req;
    if(!m_writeCh.try_dequeue(req))
        return;

    bool expected = false;
    if(!m_isWriting.compare_exchange_strong(expected, true))
    {
        m_writeCh.enqueue(req);
        return;
    }

    qDebug() << "Flushing request for task_id:" << req.task_id()
             << ", chunk_id:" << (req.has_chunk() ? req.chunk().id() : 0);
    StartWrite(&req);
}

void EmbeddingReactor::_pull()
{
    if(m_isDone.load())
        return;

    if(m_isReading.load())
    {
        qWarning() << "Already reading, cannot pull for task_id:" << m_taskId;
        return;
    }

    m_isReading.store(true);
    auto ec      = m_resp.error_code();
    auto taskId  = m_resp.task_id();
    auto chunkId = m_resp.chunk_id();

    const std::string &vectorIndexs = m_resp.vector_indexs();
    QByteArray data(vectorIndexs.data(), static_cast<int>(vectorIndexs.size()));
    qDebug() << "Received embedding result for task_id:" << m_taskId
             << "error_code=" << ec << "data size=" << data.size();

    emit GrpcClient::instance()
        -> signalEmbeddingResp(ec, taskId, chunkId, data);
    StartRead(&m_resp);
}

void EmbeddingReactor::OnWriteDone(bool ok)
{
    m_isWriting.store(false);

    if(!ok)
    {
        qWarning() << "Write failed for task_id:" << m_taskId << ", cancelling";
        return;
    }

    _flush();
}

void EmbeddingReactor::OnReadDone(bool ok)
{
    m_isReading.store(false);
    if(!ok)
    {
        qDebug() << "Read completed (no more responses) for task_id:"
                 << m_taskId;
        return;
    }

    _pull();
}

void EmbeddingReactor::OnDone(const grpc::Status &status)
{
    qDebug() << "Embedding stream finished for task_id:" << m_taskId
             << "status ok?" << status.ok()
             << "error:" << status.error_message().c_str();

    if(m_isDone.load())
    {
        qDebug() << "Stream already marked as done, not emitting signal "
                    "for task_id:"
                 << m_taskId;
        return;
    }

    if(!status.ok())
    {
        // filt some error
        if(status.error_code() != grpc::StatusCode::CANCELLED)
        {
            QString errorMsg = QString::fromStdString(status.error_message());
            emit    m_client->signalEmbeddingResp(
                ErrorCode::ERR_SERVER_DISCONNECTED,
                m_taskId,
                0,
                "");
        }
    }

    m_isDone.store(true);
    m_isWriting.store(false);
    m_isReading.store(false);
    if(m_client)
        m_client->RemoveEmbeddingReactor(m_taskId);
}

void EmbeddingReactor::OnConnectionLost()
{
    m_isWriting.store(false);
    m_isReading.store(false);
    GrpcLibraryV1::EmbeddingReq req;
    while(m_writeCh.try_dequeue(req))
    {
    }
}

// ----------------------------------- EmbeddingReactor ----------------------------

void SubscribeReactor::OnReadDone(bool ok)
{
    if(ok)
    {
        auto payload = QString::fromStdString(m_pubMsg.payload());
        auto parts   = payload.split(TOPIC_SEPARATOR, Qt::SkipEmptyParts);
        if(parts.size() < 2)
        {
            qWarning()
                << "Invalid payload format, expected 'topic|content', got:"
                << payload;
            StartRead(&m_pubMsg);
            return;
        }

        QString topic   = parts[0];
        QString content = parts[1];
        qDebug() << "Async Received PubMessage for user_id:" << m_user_id
                 << ", topic:" << topic << ", content:" << content;

        TimedQueue::instance().enqueue([topic, content]() {
            emit GrpcClient::instance() -> signalPubMessageNtf(topic, content);
        });

        StartRead(&m_pubMsg);
    }
}

void SubscribeReactor::OnDone(const grpc::Status &status)
{
    qDebug() << "Async Query stream finished for user_id:" << m_user_id
             << " Status ok? " << status.ok();

    if(!status.ok())
    {
        qWarning() << "Subscribe stream finished with error for user_id:"
                   << m_user_id << ", error:" << status.error_message().c_str();
    }

    delete this;
}

// ----------------------------------- SubscribeReactor ----------------------------