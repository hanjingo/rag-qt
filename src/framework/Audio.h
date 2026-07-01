#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QCoreApplication>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QAudioSource>
#include <QIODevice>
#include <QMap>
#include <QDebug>

class AudioStreamReceiver : public QIODevice
{
    Q_OBJECT

  public:
    explicit AudioStreamReceiver(QObject *parent = nullptr)
        : QIODevice(parent)
    {
        open(QIODevice::WriteOnly);
    }

  signals:
    void SignalAudioCaptured(const QByteArray &data);

  protected:
    qint64 writeData(const char *data, qint64 len) override
    {
        emit SignalAudioCaptured(QByteArray(data, len));
        return len;
    }

    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return 0;
    }
};

class AudioMgr : public QObject
{
    Q_OBJECT

  public:
    static AudioMgr           *Instance();
    static QList<QAudioDevice> inputs();

    void enable(const QByteArray &devId = QByteArray());

    void disable(const QByteArray &devId = QByteArray());

    bool isEnabled(const QByteArray &devId);

    qint64 startCapture(const QAudioFormat &format,
                        const QByteArray   &devId = QByteArray());

    qint64 stopCapture(qint64 id);

  signals:
    void SignalAudioDeviceEnabled(const QVector<QByteArray> &devIds);
    void SignalAudioDeviceDisable(const QVector<QByteArray> &devIds);
    void SignalAudioCaptureStarted(const qint64 id, const QByteArray &devId);
    void SignalAudioCaptured(const qint64 id, const QByteArray &data);
    void SignalAudioCaptureStopped(const qint64 id);

  public slots:
    void SlotAudioCaptureStart(const QAudioFormat &format,
                               const QByteArray   &devId);
    void SlotAudioCaptureStop(const qint64 id);

  private:
    explicit AudioMgr(QObject *parent = nullptr);
    ~AudioMgr();

  private:
    static AudioMgr *m_stAudioMgrInst;

    QMap<qint64, QAudioSource *> m_srcs;
    QMap<QByteArray, bool>       m_devs;
};

#endif // AUDIO_H