#ifndef CONFIG_H
#define CONFIG_H

#include <atomic>
#include <QObject>
#include <QVector>
#include <QPointer>
#include <QJsonArray>
#include <QJsonObject>

#include "Bus.h"

class Config : public QObject
{
    Q_OBJECT

  public:
    struct VadParam
    {
        float threshold       = 0.5;
        int   minSpeechDurMs  = 250;
        int   minSilenceDurMs = 100;
        float maxSpeechDurS   = 100.0;
        int   speechPadMs     = 30;
        float samplesOverlap  = 0.1;
    };

    struct AsrParam
    {
        QString id;

        // full param
        int      nThreads           = 4;
        int      nMaxTextCtx        = 16384;
        int      offsetMs           = 0;
        int      durationMs         = 0;
        bool     translate          = false;
        bool     detectLanguage     = false;
        QString  language           = "auto";
        bool     noCtx              = true;
        bool     noTimestamps       = false;
        bool     singleSegment      = false;
        bool     printSpecial       = false;
        bool     printProgress      = true;
        bool     printRealtime      = false;
        bool     printTimestamps    = true;
        bool     carryInitialPrompt = false;
        QString  initialPrompt      = "";
        QString  suppressRegex      = "";
        bool     suppressBlank      = true;
        bool     suppressNst        = false;
        float    temperature        = 0.0f;
        float    temperatureInc     = 0.2f;
        float    maxInitialTs       = 1.0f;
        float    lengthPenalty      = -1.0f;
        float    entropyThold       = 2.4f;
        float    logprobThold       = -1.0f;
        float    noSpeechThold      = 0.6f;
        bool     vad                = false;
        QString  vadModelPath       = "";
        VadParam vadParams;

        // buffer control
        int minNewSampleSize   = 6400;
        int minAudioBufferSize = 32000;
        int maxAudioBufferSize = 96000;
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
        int      penaltyLastN;
        float    penaltyRepeat;
        float    penaltyFreq;
        float    penaltyPresent;
        float    temperature;
        float    temperatureExt;
        float    temperatureExtDelta;
        float    temperatureExtExponent;
        uint32_t seed;
        float    topP;
        int      topPMinKeep;
        int32_t  topK;
        float    minP;
        int      minPMinKeep;

        // control parameters
        int     ctxWindowSize;
        QString stopWords;

        // prompt
        QString prompt;
    };

    struct MemoryConfig
    {
        QString id = "";
        QString indexFilePath;
        QString metaFilePath;
        QString originFilePath;
        int     dimension = -1;

        // chunk param
        int     chunkSize         = -1;
        int     overlap           = -1;
        bool    respectSentences  = false;
        bool    respectParagraphs = false;
        QString encoding;
    };

  public:
    static QPointer<Config> instance()
    {
        static QPointer<Config> inst = new Config();
        return inst;
    }
    static QString getConfigFilePath();
    static QString getCoreExeFilePath();
    static QString getCoreConfigFilePath();

    QString getPluginFilePath();
    QString getModelConfigFilePath();
    QString getMemoryConfigFilePath();
    QString getAsrConfigFilePath();

    bool         init();
    bool         load(const QString &filepath);
    bool         loadModel(const QString &filepath);
    bool         loadMemory(const QString &filepath);
    bool         loadAsr(const QString &filepath);
    bool         save(const QString &filepath);
    bool         saveModel(const QString &filepath);
    bool         saveMemory(const QString &filepath);
    bool         saveAsr(const QString &filepath);
    QJsonObject &rootObj() { return m_rootObj; }

    // bool migrate(const QString &oldFilePath, const QString &newFilePath);
    void envCompat();

    bool             isCoreRun();
    QSet<QString>    getSupportedDocTypes();
    QVector<QString> getAppUpgradeUrls();
    QVector<QString> getPluginUploadUrls();

    QVector<Bus::AudioParam>  getBusAudioParams();
    Config::AsrParam          getDefaultAsrParam();
    Config::AsrParam          getAsrParamById(const QString &id);
    QVector<Config::AsrParam> getAsrParams();
    QVector<QString>          getAsrIds();
    void                      setAsrParams(QVector<Config::AsrParam> &parmas);

    QVector<Config::NetworkConfig> networkConfigs();
    void setNetworkConfigs(QVector<Config::NetworkConfig> &configs);

    Config::ModelConfig          getModelConfigById(const QString &id);
    QVector<Config::ModelConfig> modelConfigs();
    void setModelConfigs(QVector<Config::ModelConfig> &configs);

    Config::MemoryConfig          getMemoryConfigById(const QString &id);
    QVector<Config::MemoryConfig> memoryConfigs();
    void setMemoryConfigs(QVector<Config::MemoryConfig> &configs);

  signals:
    void signalConfigUpdate();
    void signalAsrConfigUpdate();
    void signalModelConfigUpdate();
    void signalMemoryConfigUpdate();

  private:
    explicit Config(QObject *parent = nullptr);
    ~Config();

    void _convert(Config::ModelConfig &config, const QJsonObject &obj);
    void _convert(Config::AsrParam &config, const QJsonObject &obj);
    void _convert(QJsonObject &obj, const Config::AsrParam &config);

  private:
    std::atomic<bool> m_inited;

    QJsonObject m_rootObj;
    QJsonArray  m_modelArr;
    QJsonArray  m_memoryArr;
    QJsonArray  m_asrArr;

    QSet<QString> m_supportedDocTypes;
};

#endif // CONFIG_H