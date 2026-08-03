#include "AudioMgr.h"

#include <QDebug>

AudioMgr::AudioMgr(QObject *parent)
    : QObject(parent)
{
    auto devs = QMediaDevices::audioInputs();
    for(auto dev : devs)
    {
        m_devs[dev.id()] = true;
    }
}

AudioMgr::~AudioMgr()
{
    for(auto src : m_srcs.values())
    {
        src->stop();
    }
    m_srcs.clear();
}

QList<QAudioDevice> AudioMgr::inputs()
{
    return QMediaDevices::audioInputs();
}

void AudioMgr::enable(const QByteArray &devId)
{
    QVector<QByteArray> ids;
    for(auto id : m_devs.keys())
    {
        if(devId.isEmpty() || devId == id)
        {
            m_devs[id] = true;
            ids.append(id);
        }
    }

    emit signalAudioDeviceEnabled(ids);
}

void AudioMgr::disable(const QByteArray &devId)
{
    QVector<QByteArray> ids;
    for(auto id : m_devs.keys())
    {
        if(devId.isEmpty() || devId == id)
        {
            m_devs[id] = false;
            ids.append(id);
        }
    }

    emit signalAudioDeviceDisable(ids);
}

bool AudioMgr::isEnabled(const QByteArray &devId)
{
    if(m_devs.find(devId) == m_devs.end())
        return false;

    return m_devs[devId];
}

qint64 AudioMgr::startCapture(const QAudioFormat &format,
                              const QByteArray   &devId)
{
    QAudioDevice dev;
    if(devId.isEmpty())
    {
        dev = QMediaDevices::defaultAudioInput();
    } else
    {
        auto devices = QMediaDevices::audioInputs();
        for(auto item : devices)
        {
            if(item.id() != devId)
                continue;

            dev = item;
            break;
        }
    }
    if(dev.isNull())
    {
        qDebug() << "Audio device not found";
        return -1;
    }

    if(m_devs.find(dev.id()) == m_devs.end() || !m_devs[dev.id()])
    {
        qDebug() << "Audio device disabled";
        return -1;
    }

    if(!dev.isFormatSupported(format))
    {
        qDebug() << "Do not support this format";
        return -1;
    }

    auto       src      = new QAudioSource(dev, format, this);
    auto       receiver = new AudioStreamReceiver(this);
    qint64     id = static_cast<qint64>(reinterpret_cast<std::uintptr_t>(src));
    QByteArray realDevId = dev.id();
    connect(receiver,
            &AudioStreamReceiver::signalAudioCaptured,
            this,
            [this, id, realDevId](const QByteArray &data) {
                if(!isEnabled(realDevId))
                    return;

                // qDebug() << "Audio:" << " captured data:" << data;
                emit signalAudioCaptured(id, data);
            });

    src->start(receiver);
    m_srcs[id] = src;
    emit signalAudioCaptureStarted(id, realDevId);
    return id;
}

qint64 AudioMgr::stopCapture(qint64 id)
{
    if(!m_srcs.contains(id))
    {
        qDebug() << "Audio stream:" << id << " not exist";
        return -1;
    }

    auto src = m_srcs.take(id);
    if(!src)
    {
        qDebug() << "Audio stream:" << id << " closed";
        return -1;
    }

    m_srcs.remove(id);
    src->stop();
    src->deleteLater();
    emit signalAudioCaptureStopped(id);
    return id;
}

void AudioMgr::slotAudioCaptureStart(const QAudioFormat &format,
                                     const QByteArray   &devId)
{
    auto ret = startCapture(format, devId);
    qDebug() << "On slotAudioCaptureStart with ret=" << ret;
}

void AudioMgr::slotAudioCaptureStop(const qint64 id)
{
    auto ret = stopCapture(id);
    qDebug() << "On slotAudioCaptureStop id=" << id << ", ret=" << ret;
}