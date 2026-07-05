#include "GrpcClientReactor.h"

#include <QDebug>
#include <QCoreApplication>

#include "Error.h"
#include "TimedQueue.h"

void QueryReactor::OnReadDone(bool ok)
{
    if(ok)
    {
        auto ec         = m_resp.error_code();
        auto id         = m_id;
        auto content    = QString::fromStdString(m_resp.content());
        auto isFinished = m_resp.is_finished();
        qDebug() << "Async Received Query response, error_code:" << ec
                 << ", id:" << id << ", content:" << content
                 << ", is_finished:" << isFinished;

        TimedQueue::Instance().enqueue([ec, id, content, isFinished]() {
            emit GrpcClient::Instance()
                -> SignalQueryResp(ec, id, content, isFinished);
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
        emit m_client->SignalQueryResp(
            ErrorCode::ERR_SERVER_DISCONNECTED,
            m_id,
            QString::fromStdString(status.error_message()),
            true);
    }

    delete this;
}

// ----------------------------------- RecognizeReactor ----------------------------
RecognizeReactor::RecognizeReactor(GrpcClient *client, int64_t sessionId)
    : m_client(client)
    , m_sessionId(sessionId)
    , m_writeCh(10)
    , m_isWriting(false)
    , m_isDone(false)
{
    qDebug() << "RecognizeReactor created for session_id:" << sessionId;
}

RecognizeReactor::~RecognizeReactor()
{
    qDebug() << "RecognizeReactor destroyed for session_id:" << m_sessionId;
}

void RecognizeReactor::SendRequest(const GrpcLibrary::RecognizeReq &req)
{
    if(m_isDone.load())
    {
        qWarning() << "Cannot send request, stream is done for session_id:"
                   << req.session_id();
        emit m_client->SignalRecognizeResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                           "Server disconnected",
                                           true,
                                           0.0);
        return;
    }

    m_writeCh.enqueue(req);
    _flush();
}

void RecognizeReactor::_flush()
{
    if(m_isDone.load())
        return;

    if(m_isWriting.load())
        return;

    GrpcLibrary::RecognizeReq req;
    if(!m_writeCh.try_dequeue(req))
        return;

    m_isWriting.store(true);
    qDebug() << "Flushing request for session_id:" << req.session_id()
             << ", ctx_id:" << req.ctx_id().c_str()
             << ", has_param:" << req.has_param()
             << ", has_audio:" << req.has_audio_chunk();
    StartWrite(&req);
}

void RecognizeReactor::_pull()
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

    TimedQueue::Instance().enqueue([ec, transcript, isFinished, confidence]() {
        emit GrpcClient::Instance()
            -> SignalRecognizeResp(ec, transcript, isFinished, confidence);
    });

    if(!isFinished && !m_isDone.load())
    {
        StartRead(&m_resp);
    } else if(isFinished)
    {
        m_isDone.store(true);
    }
}

void RecognizeReactor::OnWriteDone(bool ok)
{
    m_isWriting.store(false);

    if(!ok)
    {
        qWarning() << "Write failed for session_id:" << m_sessionId
                   << ", cancelling";
        m_isDone.store(true);

        emit m_client->SignalRecognizeResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                           "Write failed - connection lost",
                                           true,
                                           0.0);
        m_client->RemoveRecognizeReactor(GetSessionId());
        return;
    }

    _flush();
}

void RecognizeReactor::OnReadDone(bool ok)
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

void RecognizeReactor::OnDone(const grpc::Status &status)
{
    qDebug() << "Recognize stream finished for session_id:" << m_sessionId
             << "status ok?" << status.ok()
             << "error:" << status.error_message().c_str();

    if(!status.ok())
    {
        QString errorMsg = QString::fromStdString(status.error_message());
        emit m_client->SignalRecognizeResp(ErrorCode::ERR_SERVER_DISCONNECTED,
                                           errorMsg,
                                           true,
                                           0.0);
    }

    m_isDone.store(true);
    m_isWriting.store(false);
    m_client->RemoveRecognizeReactor(GetSessionId());
}

void RecognizeReactor::OnConnectionLost()
{
    m_isDone.store(true);
    m_isWriting.store(false);
    GrpcLibrary::RecognizeReq req;
    while(m_writeCh.try_dequeue(req))
    {
    }
}