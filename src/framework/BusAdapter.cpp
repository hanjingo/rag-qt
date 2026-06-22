#include <QDebug>

#include "BusAdapter.h"
#include "Account.h"
#include "Error.h"

BusAdapter *BusAdapter::m_stBusAdapterInst = nullptr;
BusAdapter *BusAdapter::Instance()
{
    if(nullptr == m_stBusAdapterInst)
        m_stBusAdapterInst = new BusAdapter();

    return m_stBusAdapterInst;
}

BusAdapter::BusAdapter(QObject *parent)
    : QObject(parent)
{
    connect(Bus::Instance(), &Bus::SignalPong, this, &BusAdapter::_slotPong);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalNewSessionResp,
            this,
            &BusAdapter::_slotNewSessionResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalQueryResp,
            this,
            &BusAdapter::_slotQueryResp);

    connect(Bus::Instance(), &Bus::SignalPong, this, &BusAdapter::_slotPong);

    connect(Bus::Instance(), &Bus::SignalQuery, this, &BusAdapter::_slotQuery);

    connect(Bus::Instance(),
            &Bus::SignalNewSession,
            this,
            &BusAdapter::_slotNewSession);
}

BusAdapter::~BusAdapter()
{
}

void BusAdapter::_slotPong()
{
    qDebug() << "Received Pong signal from Bus.";
}

void BusAdapter::_slotNewSession(const QString &title,
                                 const QString &content,
                                 const QString &model)
{
    qDebug() << "Received NewSession signal from Bus. title: " << title
             << ", content: " << content << ", model: " << model;
    GrpcClient::Instance()->NewSession(Account::Instance()->Id(),
                                       Account::Instance()->Auth(),
                                       title,
                                       content,
                                       model);
}

void BusAdapter::_slotNewSessionResp(const int                     errorCode,
                                     const ::GrpcLibrary::Session &session)
{
    qDebug() << "FrameworkWidget:Received NewSessionResp signal from remote. "
                "errorCode: "
             << errorCode << ", title: " << session.title();
    if(errorCode != ErrorCode::OK)
    {
        emit Bus::Instance() -> SignalNewSessionResp(
            session.id(),
            QString::fromStdString(session.title()),
            tr("Failed to create new session with error code: %1")
                .arg(errorCode));
        return;
    }

    emit Bus::Instance()
        -> SignalNewSessionResp(session.id(),
                                QString::fromStdString(session.title()),
                                QString::fromStdString(session.content()));
}

void BusAdapter::_slotQuery(const int64_t  sessionId,
                            const QString &query,
                            const QString &model)
{
    qDebug() << "Received Bus Query signal from Bus. sessionId: " << sessionId
             << ", query: " << query;
    GrpcClient::Instance()->Query(sessionId,
                                  Account::Instance()->Id(),
                                  Account::Instance()->Auth(),
                                  query,
                                  model);
}

void BusAdapter::_slotQueryResp(const int      errorCode,
                                const int64_t  sessionId,
                                const QString &content)
{
    qDebug() << "Received Bus QueryResp signal from Bus. sessionId: "
             << sessionId << ", content: " << content;
    if(errorCode != ErrorCode::OK)
    {
        emit Bus::Instance() -> SignalQueryResp(
            sessionId,
            tr("Query failed with error code: %1").arg(errorCode));
        return;
    }

    // Forward the query response to plugins
    emit Bus::Instance() -> SignalQueryResp(sessionId, content);
}