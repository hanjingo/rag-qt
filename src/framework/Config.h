#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QVector>
#include <QJsonArray>
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
        bool    printTimestamps    = false;
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
        float wakeupThreshold     = 0.001;
        int   maxSameContentCount = 3;
    };

    struct NetworkConfig
    {
        QString ip;
        int     port;
        bool    isEnable;
    };

    struct ModelConfig
    {
        // base info
        QString id;
        QString name;
        QString publisher;
        QString timestamp;
        QString addr;
        QString pipeline;
        float   cost;
        QString apiKey;
        QString hash;

        // model params
        QString mainGPU;
        bool    vocabOnly;
        bool    useMMap;
        bool    useDirectIO;
        bool    useMLock;
        bool    checkTensors;
        bool    useExtraBufTypes;
        bool    noHost;
        bool    noAlloc;

        // context params
        int nCtx;
        int nBatch;
        int nUbatch;
        int nSeqMax;
        int nThreads;
        int nThreadsBatch;

        float ropeFreqBase;
        float ropeFreqScale;
        float yarnExtFactor;
        float yarnAttnFactor;
        float yarnBetaFast;
        float yarnBetaSlow;
        int   yarnOrigCtx;
        float defragThold;

        bool embeddings;
        bool offloadKQV;
        bool noPerf;
        bool opOffload;
        bool swaFull;
        bool kvUnified;

        // sampling parameters
        float temperature;
        float topP;
        float topK;
        float reputationPenalty;
        float minP;

        // control parameters
        int     ctxWindowSize;
        QString stopWords;

        // prompt
        QString prompt;
    };

  public:
    static Config &Instance();

    void         load(const QString &filepath);
    void         loadModel(const QString &filepath);
    void         save(const QString &filepath);
    void         saveModel(const QString &filepath);
    QJsonObject &rootObj() { return m_rootObj; }

    Config::TranslatorParam getAudioTranslatorParamById(const QString &id);
    QVector<Config::TranslatorParam> audioTranslatorParams();
    void setAudioTranslatorParams(QVector<Config::TranslatorParam> &parmas);

    QVector<Config::NetworkConfig> networkConfigs();
    void setNetworkConfigs(QVector<Config::NetworkConfig> &configs);

    Config::ModelConfig          getModelConfigById(const QString &id);
    QVector<Config::ModelConfig> modelConfigs();
    void setModelConfigs(QVector<Config::ModelConfig> &configs);

  private:
    explicit Config(QObject *parent = nullptr);
    ~Config();

  private:
    QJsonObject m_rootObj;
    QJsonArray  m_modelArr;
};

#endif // CONFIG_H