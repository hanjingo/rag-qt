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
        QString language;
        bool    translate;

        int   muteAmplitudeDurationMs = 200;
        float muteAmplitudeThreshold  = 0.03;

        int minNewSampleSize   = 6400;
        int minAudioBufferSize = 32000;
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

    QVector<Config::TranslatorParam> translatorParams();
    QVector<Config::NetworkConfig>   networkConfigs();

  private:
    explicit Config(QObject *parent = nullptr);
    ~Config();

  private:
    QJsonObject m_rootObj;
};

#endif // CONFIG_H