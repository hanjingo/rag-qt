#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QVector>
#include <QJsonArray>
#include <QJsonObject>

#include "Bus.h"

class Config : public QObject
{
    Q_OBJECT

  public:
    struct TranslatorParam
    {
        QString id;

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
    static Config &Instance();

    static QString getPluginFilePath();
    static QString getConfigFilePath();
    static QString getModelConfigFilePath();
    static QString getMemoryConfigFilePath();

    bool         load(const QString &filepath);
    bool         loadModel(const QString &filepath);
    bool         loadMemory(const QString &filepath);
    bool         save(const QString &filepath);
    bool         saveModel(const QString &filepath);
    bool         saveMemory(const QString &filepath);
    QJsonObject &rootObj() { return m_rootObj; }

    bool             isCoreRun();
    QSet<QString>    getSupportedDocTypes();
    QVector<QString> getAppUpgradeUrls();
    QVector<QString> getPluginUploadUrls();

    QVector<Bus::AudioParam> getBusAudioParams();
    Config::TranslatorParam  getAudioTranslatorParamById(const QString &id);
    QVector<Config::TranslatorParam> audioTranslatorParams();
    void setAudioTranslatorParams(QVector<Config::TranslatorParam> &parmas);

    QVector<Config::NetworkConfig> networkConfigs();
    void setNetworkConfigs(QVector<Config::NetworkConfig> &configs);

    Config::ModelConfig          getModelConfigById(const QString &id);
    QVector<Config::ModelConfig> modelConfigs();
    void setModelConfigs(QVector<Config::ModelConfig> &configs);

    QString                       getDefaultIndexPath();
    Config::MemoryConfig          getMemoryConfigById(const QString &id);
    QVector<Config::MemoryConfig> memoryConfigs();
    void setMemoryConfigs(QVector<Config::MemoryConfig> &configs);

  private:
    explicit Config(QObject *parent = nullptr);
    ~Config();

    void _convert(Config::ModelConfig &config, const QJsonObject &obj);

  private:
    QJsonObject m_rootObj;
    QJsonArray  m_modelArr;
    QJsonArray  m_memoryArr;

    QSet<QString> m_supportedDocTypes;
};

#endif // CONFIG_H