#include <QDebug>
#include <QCoreApplication>

#include <libqt/io/file.h>

#include "BusAdapter.h"
#include "Account.h"
#include "Error.h"
#include "AudioMgr.h"
#include "MemMgr.h"

BusAdapter::BusAdapter(QObject *parent)
    : QObject(parent)
{
    // from framework
    connect(this, &BusAdapter::signalPing, Bus::instance(), &Bus::signalPing);
    connect(this,
            &BusAdapter::signalModelInfoUpdateNtf,
            Bus::instance(),
            &Bus::signalModelInfoUpdateNtf);
    connect(this,
            &BusAdapter::signalMemoryInfoUpdateNtf,
            Bus::instance(),
            &Bus::signalMemoryInfoUpdateNtf);

    connect(this,
            &BusAdapter::signalAudioParamUpdateNtf,
            Bus::instance(),
            &Bus::signalAudioParamUpdateNtf);

    connect(this,
            &BusAdapter::signalRetrieveResp,
            Bus::instance(),
            &Bus::signalRetrieveResp);

    connect(GrpcClient::instance(),
            &GrpcClient::signalQueryResp,
            Bus::instance(),
            &Bus::signalQueryResp,
            Qt::QueuedConnection);

    connect(GrpcClient::instance(),
            &GrpcClient::signalStopAnswerResp,
            Bus::instance(),
            &Bus::signalStopAnswerResp);

    connect(GrpcClient::instance(),
            &GrpcClient::signalNewSessionResp,
            Bus::instance(),
            &Bus::signalNewSessionResp);

    connect(GrpcClient::instance(),
            &GrpcClient::signalGetSessionResp,
            Bus::instance(),
            &Bus::signalGetSessionResp);

    connect(GrpcClient::instance(),
            &GrpcClient::signalDelSessionResp,
            Bus::instance(),
            &Bus::signalDelSessionResp);

    connect(GrpcClient::instance(),
            &GrpcClient::signalGetChatMessageResp,
            Bus::instance(),
            &Bus::signalGetChatMessageResp);

    connect(GrpcClient::instance(),
            &GrpcClient::signalRecognizeResp,
            Bus::instance(),
            &Bus::signalRecognizeResp);

    connect(GrpcClient::instance(),
            &GrpcClient::signalStopRecognizeResp,
            Bus::instance(),
            &Bus::signalStopRecognizeResp);

    connect(GrpcClient::instance(),
            &GrpcClient::signalUploadResp,
            Bus::instance(),
            &Bus::signalUploadResp);

    connect(AudioMgr::instance(),
            &AudioMgr::signalAudioCaptureStarted,
            Bus::instance(),
            &Bus::signalAudioCaptureStarted);

    connect(AudioMgr::instance(),
            &AudioMgr::signalAudioCaptured,
            Bus::instance(),
            &Bus::signalAudioCaptured);

    connect(AudioMgr::instance(),
            &AudioMgr::signalAudioCaptureStopped,
            Bus::instance(),
            &Bus::signalAudioCaptureStopped);

    // from plugin
    connect(Bus::instance(), &Bus::signalPong, this, &BusAdapter::_slotPong);

    connect(Bus::instance(),
            &Bus::signalQuery,
            this,
            &BusAdapter::_slotQueryFromBus);

    connect(Bus::instance(),
            &Bus::signalStopAnswer,
            this,
            &BusAdapter::_slotStopAnswerFromBus);

    connect(Bus::instance(),
            &Bus::signalGetSession,
            this,
            &BusAdapter::_slotGetSessionFromBus);


    connect(Bus::instance(),
            &Bus::signalNewSession,
            this,
            &BusAdapter::_slotNewSessionFromBus);

    connect(Bus::instance(),
            &Bus::signalGetChatMessage,
            this,
            &BusAdapter::_slotGetChatMessageFromBus);

    connect(Bus::instance(),
            &Bus::signalRecognize,
            this,
            &BusAdapter::_slotAudioTranslate);

    connect(Bus::instance(),
            &Bus::signalStopRecognize,
            this,
            &BusAdapter::_slotAudioStopTranslate);

    connect(Bus::instance(),
            &Bus::signalUpload,
            this,
            &BusAdapter::_slotUploadFromBus);

    connect(Bus::instance(),
            &Bus::signalRetrieve,
            this,
            &BusAdapter::_slotRetrieveFromBus);

    connect(Bus::instance(),
            &Bus::signalAudioCaptureStart,
            AudioMgr::instance(),
            &AudioMgr::slotAudioCaptureStart);

    connect(Bus::instance(),
            &Bus::signalAudioCaptureStop,
            AudioMgr::instance(),
            &AudioMgr::slotAudioCaptureStop);
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
    GrpcClient::instance()->NewSession(Account::instance()->id(),
                                       Account::instance()->auth(),
                                       title,
                                       content,
                                       model);
}

void BusAdapter::_slotDelSessionFromBus(const QVector<int64_t> &ids)
{
    qDebug() << "Received DelSession signal from Bus. ids: " << ids;
    GrpcClient::instance()->DelSession(Account::instance()->id(),
                                       Account::instance()->auth(),
                                       ids);
}

void BusAdapter::_slotQueryFromBus(const int64_t         sessionId,
                                   const int64_t         msgId,
                                   const QString        &query,
                                   const QString        &model,
                                   const Bus::ModelInfo &info)
{
    qDebug() << "Received Bus Query signal from Bus. sessionId: " << sessionId
             << ", msgId: " << msgId << ", query: " << query;
    auto conf          = Config::instance()->getModelConfigById(info.id);
    conf.pipeline      = info.pipeline;
    conf.hash          = info.hash;
    conf.ctxWindowSize = info.ctxWindowSize;
    conf.stopWords     = info.stopWords;
    conf.prompt        = info.prompt;
    GrpcClient::instance()->Query(sessionId,
                                  msgId,
                                  Account::instance()->id(),
                                  Account::instance()->auth(),
                                  query,
                                  model,
                                  conf);
}

void BusAdapter::_slotStopAnswerFromBus(const int64_t sessionId)
{
    qDebug() << "Received Bus StopAnswer signal from Bus. sessionId: "
             << sessionId;
    GrpcClient::instance()->StopAnswer(sessionId,
                                       Account::instance()->id(),
                                       Account::instance()->auth());
}

void BusAdapter::_slotGetSessionFromBus(const int64_t id, int limit)
{
    qDebug() << "Received Bus GetSession signal from Bus. id: " << id
             << ", user_id: " << Account::instance()->id()
             << ", limit: " << limit;
    GrpcClient::instance()->GetSession(id,
                                       Account::instance()->id(),
                                       Account::instance()->auth(),
                                       limit);
}

void BusAdapter::_slotGetChatMessageFromBus(const int64_t msgId,
                                            const int64_t sessionId,
                                            int           limit)
{
    qDebug() << "Received Bus GetChatMessage signal from Bus. sessionId: "
             << sessionId << ", user_id: " << Account::instance()->id()
             << ", msg_id: " << msgId << ", limit: " << limit;
    GrpcClient::instance()->GetChatMessage(sessionId,
                                           Account::instance()->id(),
                                           Account::instance()->auth(),
                                           msgId,
                                           limit);
}

void BusAdapter::_slotAudioTranslate(const qint64      sessionId,
                                     const QByteArray &src,
                                     const QString    &translatorId)
{
    qDebug() << "Receive Bus Audio Translate signal from Bus. session_id: "
             << sessionId << ", translatorId: " << translatorId;

    auto param = Config::instance()->getAsrParamById(translatorId);
    if(param.id.isEmpty())
        param = Config::instance()->getDefaultAsrParam();

    GrpcClient::instance()->Recognize(sessionId,
                                      Account::instance()->id(),
                                      Account::instance()->auth(),
                                      src,
                                      param,
                                      param.id);
}

void BusAdapter::_slotAudioStopTranslate(const qint64 sessionId)
{
    qDebug() << "Receive Bus Audio Stop Translate signal from Bus. session_id: "
             << sessionId;

    GrpcClient::instance()->RecognizeStop(sessionId,
                                          Account::instance()->id(),
                                          Account::instance()->auth());
}

void BusAdapter::_slotUploadFromBus(const QString &filePath)
{
    qDebug() << "Receive Bus Upload signal from Bus. filePath: " << filePath;
    if(File::isExist(filePath) == false)
    {
        qDebug() << "File does not exist: " << filePath;
        emit Bus::instance()
            -> signalUploadResp(ErrorCode::ERR_FILE_NOT_FOUND, "");
        return;
    }

    GrpcClient::instance()->Upload(File::md5(filePath),
                                   Account::instance()->id(),
                                   Account::instance()->auth(),
                                   filePath,
                                   File::fileSizeKB(filePath));
}

void BusAdapter::_slotRetrieveFromBus(const QString &question,
                                      const int      topK,
                                      const QString &memoryId)
{
    qDebug() << "Receive Bus Retrieve signal from Bus. question: " << question
             << ", topK: " << topK << ", memoryId: " << memoryId;

    MemMgr::instance()->retrieve(question.toStdString(),
                                 topK,
                                 memoryId.toStdString());
}