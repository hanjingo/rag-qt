#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QVector>
#include <QJsonObject>

class Config : public QObject
{
    Q_OBJECT

  public:
    struct TranslatorParam
    {
        QString id;
        QString modelPath;

        // ctx param
        bool useGPU         = false;
        int  gpuDevice      = 0;
        bool flashAttention = true;

        // full param
        int     nThreads           = 4;
        int     nMaxTextCtx        = 16384;
        int     offsetMs           = 0;
        int     durationMs         = 0;
        bool    translate          = false;
        bool    detectLanguage     = true;
        QString language           = "auto";
        bool    noCtx              = true;
        bool    noTimestamps       = false;
        bool    singleSegment      = false;
        bool    printSpecial       = false;
        bool    printProgress      = false;
        bool    printRealtime      = false;
        bool    carryInitialPrompt = false;
        QString initialPrompt      = "";
        QString suppressRegex      = "";
        bool    suppressBlank      = true;
        bool    suppressNst        = false;
        float   temperature        = 0.0;
        float   temperatureInc     = 0.2;
        float   maxInitialTs       = 1.0;
        float   lengthPenalty      = -1.0;
        float   entropyThold       = 2.4;
        float   logprobThold       = -1.0;
        float   noSpeechThold      = 0.6;

        // mute control
        int   minNewSampleSize        = 6400;
        int   minAudioBufferSize      = 32000;
        int   muteAmplitudeDurationMs = 200;
        float muteAmplitudeThreshold  = 0.03;
        int   keepLastAudioBufferMs   = 1000;

        // sleep and wake up control
        int   sleepTimeoutMs      = 3000;
        float wakeupThreshold     = 0.01;
        int   maxSameContentCount = 3;

        // noise control
        QString     filtRegex;
        QStringList noiseWords;
    };

    struct NetworkConfig
    {
        QString ip;
        int     port;
        bool    isEnable;
    };

  public:
    static Config &Instance();

    void         load(const QString &filepath);
    void         save(const QString &filepath);
    QJsonObject &rootObj() { return m_rootObj; }

    QVector<Config::TranslatorParam> audioTranslatorParams();
    void setAudioTranslatorParams(QVector<Config::TranslatorParam> &parmas);

    QVector<Config::NetworkConfig> networkConfigs();
    void setNetworkConfigs(QVector<Config::NetworkConfig> &configs);

  private:
    explicit Config(QObject *parent = nullptr);
    ~Config();

  private:
    QJsonObject m_rootObj;
};

#endif // CONFIG_H