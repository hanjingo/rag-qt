#ifndef AUDIOMGR_H
#define AUDIOMGR_H

#include <QMap>
#include <QByteArray>
#include <QAudioDevice>
#include <QAudioFormat>
#include <QAudioSource>
#include <QPointer>

#include <libqt/multimedia/AudioStreamReceiver.h>

class AudioMgr : public QObject
{
    Q_OBJECT

  public:
    static QPointer<AudioMgr> instance()
    {
        static QPointer<AudioMgr> inst = new AudioMgr();
        return inst;
    }
    static QList<QAudioDevice> inputs();

    void enable(const QByteArray &devId = QByteArray());

    void disable(const QByteArray &devId = QByteArray());

    bool isEnabled(const QByteArray &devId);

    qint64 startCapture(const QAudioFormat &format,
                        const QByteArray   &devId = QByteArray());

    qint64 stopCapture(qint64 id);

  signals:
    void signalAudioDeviceEnabled(const QVector<QByteArray> &devIds);
    void signalAudioDeviceDisable(const QVector<QByteArray> &devIds);
    void signalAudioCaptureStarted(const qint64 id, const QByteArray &devId);
    void signalAudioCaptured(const qint64        id,
                             const QAudioFormat &format,
                             const QByteArray   &data);
    void signalAudioCaptureStopped(const qint64 id);

  public slots:
    void slotAudioCaptureStart(const QAudioFormat &format,
                               const QByteArray   &devId);
    void slotAudioCaptureStop(const qint64 id);

  private:
    explicit AudioMgr(QObject *parent = nullptr);
    ~AudioMgr();

  private:
    QMap<qint64, QAudioSource *> m_srcs;
    QMap<QByteArray, bool>       m_devs;
};

#endif // AUDIOMGR_H