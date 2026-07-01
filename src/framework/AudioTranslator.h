#ifndef AUDIOTRANSLATOR_H
#define AUDIOTRANSLATOR_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QThread>

#include <hj/ai/asr.hpp>

class AudioTranslator : public QObject
{
    Q_OBJECT

  public:
    explicit AudioTranslator(const QString               &modelPath,
                             const hj::asr::ctx_params_t &params,
                             QObject                     *parent = nullptr);
    explicit AudioTranslator(const QString                &modelPath,
                             const hj::asr::ctx_params_t  &params,
                             const hj::asr::full_params_t &fullParams,
                             QObject                      *parent = nullptr);
    ~AudioTranslator();

    // QByteArray to std::vector<float> (pcmf32)
    static void convert(std::vector<float> &pcmf32, const QByteArray &data);

    hj::asr::full_params_t fullParams();

    void     setWorkerThread(QThread *thread) { m_thread = thread; }
    QThread *workerThread() const { return m_thread; }

    void  setMuteAmplitudeDurationMs(int durMs);
    float muteAmplitudeDurationMs() const { return m_muteAmplitudeDurationMs; }
    void  setMuteAmplitudeThreshold(float threshold);
    float muteAmplitudeThreshold() const { return m_muteAmplitudeThreshold; }

    void setMinAudioBufferSize(int size);
    int  minAudioBufferSize() const { return m_minAudioBufferSize; }
    void setMinNewSampleSize(int size);
    int  minNewSampleSize() const { return m_minNewSampleSize; }

    bool checkCurrSegmentFinished(const QString &catched);
    void checkAmplitude(int lastMs, float threshold);
    bool checkBufSize();
    bool checkNewSampleSize(int newSampleSize);
    void filt(QString &text);

    int translate(QVector<QString> &segments, const QByteArray &data);
    int translate(QVector<QString>             &segments,
                  const QByteArray             &data,
                  const hj::asr::full_params_t &params);

  signals:
    void SignalTranslationFinished(const int               errorCode,
                                   const QByteArray       &src,
                                   const QVector<QString> &segments);

  public slots:
    void SlotProcessAudio(const QByteArray             &data,
                          const hj::asr::full_params_t &params);

  private:
    QThread *m_thread = nullptr;

    hj::asr::context       m_ctx;
    hj::asr::full_params_t m_fullParams;
    std::vector<float>     m_pcmBuf;

    int   m_muteAmplitudeDurationMs = 200;
    float m_muteAmplitudeThreshold  = 0.0;

    int m_minNewSampleSize   = 6400;
    int m_minAudioBufferSize = 32000;

    int              m_muteCount     = 0;
    int              m_newSampleSize = 0;
    QVector<QString> m_history;
};

class AudioTranslatorMgr : public QObject
{
    Q_OBJECT

  public:
    explicit AudioTranslatorMgr(QObject *parent = nullptr);
    ~AudioTranslatorMgr();

    static AudioTranslatorMgr *Instance();

    AudioTranslator *Create(const QString               &id,
                            const QString               &modelPath,
                            const hj::asr::ctx_params_t &params);
    AudioTranslator *Create(const QString                &id,
                            const QString                &modelPath,
                            const hj::asr::ctx_params_t  &params,
                            const hj::asr::full_params_t &fullParams);

    AudioTranslator *Get(const QString &id = "");

    int Translate(const QByteArray &src, const QString &id);
    int Translate(const QByteArray            &src,
                  const QString               &id,
                  const hj::asr::full_params_t params);

  signals:
    void SignalAudioTranslated(const int               errorCode,
                               const QByteArray       &src,
                               const QVector<QString> &segments);
    void SignalProcessAudio(const QByteArray             &data,
                            const hj::asr::full_params_t &params);

  public slots:
    void SlotAudioTranslate(const QByteArray &src, const QString &id);

  private:
    QMap<QString, AudioTranslator *> m_translators;
};

#endif