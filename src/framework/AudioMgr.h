#ifndef AUDIOMGR_H
#define AUDIOMGR_H

#include <QMap>
#include <QByteArray>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSource>

#include <libqt/multimedia/AudioStreamReceiver.h>

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
    QMap<qint64, QAudioSource *> m_srcs;
    QMap<QByteArray, bool>       m_devs;
};

#endif // AUDIOMGR_H