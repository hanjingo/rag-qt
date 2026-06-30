#include "GrpcClientReactor.h"

#include <QDebug>
#include <QCoreApplication>

#include "Error.h"

void QueryReactor::OnReadDone(bool ok)
{
    if(ok)
    {
        qDebug() << "Async Received Query response, error_code:"
                 << m_resp.error_code() << ", id:" << m_id
                 << ", content:" << QString::fromStdString(m_resp.content())
                 << ", is_finished:" << m_resp.is_finished();

        emit m_client->SignalQueryResp(m_resp.error_code(),
                                       m_id,
                                       QString::fromStdString(m_resp.content()),
                                       m_resp.is_finished());
        QCoreApplication::processEvents();
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