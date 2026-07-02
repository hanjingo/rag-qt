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

    void initSleepControl();

    // QByteArray to std::vector<float> (pcmf32)
    static void convert(std::vector<float> &pcmf32, const QByteArray &data);

    void     setWorkerThread(QThread *thread) { m_thread = thread; }
    QThread *workerThread() const { return m_thread; }

    // full params
    hj::asr::full_params_t fullParams();
    void                   setFullParams(const hj::asr::full_params_t &params);

    // mute control
    void  setMinAudioBufferSize(int size);
    int   minAudioBufferSize() const { return m_minAudioBufferSize; }
    void  setMinNewSampleSize(int size);
    int   minNewSampleSize() const { return m_minNewSampleSize; }
    void  setMuteAmplitudeDurationMs(int durMs);
    float muteAmplitudeDurationMs() const { return m_muteAmplitudeDurationMs; }
    void  setMuteAmplitudeThreshold(float threshold);
    float muteAmplitudeThreshold() const { return m_muteAmplitudeThreshold; }
    void  setKeepLastAudioBufferMs(int ms);
    int   keepLastAudioBufferMs() const { return m_keepLastAudioBufferMs; }

    // noise control
    void setFiltRegex(const QString &regex);
    void setNoiseWords(const QVector<QString> &words) { m_noiseWords = words; }

    // sleep and wake up control
    void  setSleepTimeoutMs(int ms);
    int   sleepTimeoutMs() const { return m_sleepTimeoutMs; }
    void  setWakeupThreshold(float threshold);
    float wakeupThreshold() const { return m_wakeupThreshold; }
    void  setMaxSameContentCount(int count);
    int   maxSameContentCount() const { return m_maxSameContentCount; }

    // tool function
    bool checkCurrSegmentFinished(const QString &catched);
    void checkAmplitude();
    bool checkBufSize();
    bool checkNewSampleSize(int newSampleSize);
    void filt(QString &text);

    bool  shouldWakeUp();
    void  trySleep();
    bool  isSleeping() const { return m_isSleeping; }
    void  wakeUp();
    bool  hasAudioActivity();
    float getCurrentAmplitude();

    int translate(QVector<QString> &segments, const QByteArray &data);
    int translate(QVector<QString>             &segments,
                  const QByteArray             &data,
                  const hj::asr::full_params_t &params);

  signals:
    void SignalTranslationFinished(const int               errorCode,
                                   const QByteArray       &src,
                                   const QVector<QString> &segments);
    void SignalTranslatorSlept();
    void SignalTranslatorWokeUp();

  public slots:
    void SlotProcessAudio(const QByteArray             &data,
                          const hj::asr::full_params_t &params);
    void SlotCheckSleep();

  private:
    QThread *m_thread = nullptr;

    hj::asr::context       m_ctx;
    hj::asr::full_params_t m_fullParams;
    std::vector<float>     m_pcmBuf;

    bool m_isActive = true;

    int   m_muteAmplitudeDurationMs = 200;
    float m_muteAmplitudeThreshold  = 0.0;

    int m_minNewSampleSize      = 6400;
    int m_minAudioBufferSize    = 32000;
    int m_keepLastAudioBufferMs = 1000;

    QString          m_filtRegex;
    QVector<QString> m_noiseWords;

    int m_muteCount     = 0;
    int m_newSampleSize = 0;

    bool          m_isSleeping       = false;
    int           m_silentFrameCount = 0;
    int           m_sleepTimeoutMs   = 3000;
    float         m_wakeupThreshold  = 0.01;
    QTimer       *m_sleepCheckTimer  = nullptr;
    QElapsedTimer m_lastActivityTimer;
    bool          m_hasEverHadActivity = false;

    QString m_lastTranslation;
    int     m_sameContentCount    = 0;
    int     m_maxSameContentCount = 3;
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