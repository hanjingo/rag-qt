#include <QDebug>
#include <QCoreApplication>

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
    // from framework
    connect(this, &BusAdapter::SignalPing, Bus::Instance(), &Bus::SignalPing);
    connect(this,
            &BusAdapter::SignalModelInfoUpdateNtf,
            Bus::Instance(),
            &Bus::SignalModelInfoUpdateNtf);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalQueryResp,
            Bus::Instance(),
            &Bus::SignalQueryResp,
            Qt::QueuedConnection);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalStopAnswerResp,
            Bus::Instance(),
            &Bus::SignalStopAnswerResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalNewSessionResp,
            Bus::Instance(),
            &Bus::SignalNewSessionResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalGetSessionResp,
            Bus::Instance(),
            &Bus::SignalGetSessionResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalDelSessionResp,
            Bus::Instance(),
            &Bus::SignalDelSessionResp);

    connect(GrpcClient::Instance(),
            &GrpcClient::SignalGetMessageInfoResp,
            Bus::Instance(),
            &Bus::SignalGetMessageInfoResp);

    // from plugin
    connect(Bus::Instance(), &Bus::SignalPong, this, &BusAdapter::_slotPong);

    connect(Bus::Instance(),
            &Bus::SignalQuery,
            this,
            &BusAdapter::_slotQueryFromBus);

    connect(Bus::Instance(),
            &Bus::SignalStopAnswer,
            this,
            &BusAdapter::_slotStopAnswerFromBus);

    connect(Bus::Instance(),
            &Bus::SignalGetSession,
            this,
            &BusAdapter::_slotGetSessionFromBus);

    connect(Bus::Instance(),
            &Bus::SignalNewSession,
            this,
            &BusAdapter::_slotNewSessionFromBus);

    connect(Bus::Instance(),
            &Bus::SignalGetMessageInfo,
            this,
            &BusAdapter::_slotGetMessageInfoFromBus);
}

BusAdapter::~BusAdapter()
{
}

void BusAdapter::_slotPong()
{
    qDebug() << "Received Pong signal from Bus.";
    // TODO heartbeat
}

void BusAdapter::_slotNewSessionFromBus(const QString &title,
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

void BusAdapter::_slotDelSessionFromBus(const QVector<int64_t> &ids)
{
    qDebug() << "Received DelSession signal from Bus. ids: " << ids;
    GrpcClient::Instance()->DelSession(Account::Instance()->Id(),
                                       Account::Instance()->Auth(),
                                       ids);
}

void BusAdapter::_slotQueryFromBus(const int64_t           sessionId,
                                   const QString          &query,
                                   const QString          &model,
                                   const Bus::ModelConfig &config)
{
    qDebug() << "Received Bus Query signal from Bus. sessionId: " << sessionId
             << ", query: " << query;
    GrpcClient::Instance()->Query(sessionId,
                                  Account::Instance()->Id(),
                                  Account::Instance()->Auth(),
                                  query,
                                  model,
                                  config);
}

void BusAdapter::_slotStopAnswerFromBus(const int64_t sessionId)
{
    qDebug() << "Received Bus StopAnswer signal from Bus. sessionId: "
             << sessionId;
    GrpcClient::Instance()->StopAnswer(sessionId,
                                       Account::Instance()->Id(),
                                       Account::Instance()->Auth());
}

void BusAdapter::_slotGetSessionFromBus(const int64_t id, int limit)
{
    qDebug() << "Received Bus GetSession signal from Bus. id: " << id
             << ", user_id: " << Account::Instance()->Id()
             << ", auth: " << Account::Instance()->Auth()
             << ", limit: " << limit;
    GrpcClient::Instance()->GetSession(id,
                                       Account::Instance()->Id(),
                                       Account::Instance()->Auth(),
                                       limit);
}

void BusAdapter::_slotGetMessageInfoFromBus(const int64_t msgId,
                                            const int64_t sessionId,
                                            int           limit)
{
    qDebug() << "Received Bus GetMessageInfo signal from Bus. sessionId: "
             << sessionId << ", user_id: " << Account::Instance()->Id()
             << ", auth: " << Account::Instance()->Auth()
             << ", msg_id: " << msgId << ", limit: " << limit;
    GrpcClient::Instance()->GetMessageInfo(sessionId,
                                           Account::Instance()->Id(),
                                           Account::Instance()->Auth(),
                                           msgId,
                                           limit);
}