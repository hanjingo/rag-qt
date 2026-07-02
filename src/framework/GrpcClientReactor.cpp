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