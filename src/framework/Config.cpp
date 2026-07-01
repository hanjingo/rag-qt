#include "Config.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileDialog>
#include <QSaveFile>

#include "Global.h"

Config::Config(QObject *parent)
    : QObject(parent)
{
    load("./config.json");
}

Config::~Config()
{
}

Config &Config::Instance()
{
    static Config instance;
    return instance;
}

void Config::load(const QString &filepath)
{
    QFile         readFile(filepath);
    QJsonDocument doc;
    if(readFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray      data = readFile.readAll();
        QJsonParseError parseError;
        doc = QJsonDocument::fromJson(data, &parseError);
        if(parseError.error != QJsonParseError::NoError || !doc.isObject())
        {
            qDebug() << "Failed to parse JSON from config file: "
                     << parseError.errorString();
            return;
        }
        readFile.close();
    } else
    {
        qDebug() << "Failed to open config file for reading: "
                 << readFile.errorString();
        return;
    }

    m_rootObj = doc.object();
}

void Config::save(const QString &filepath)
{
    QJsonDocument doc(m_rootObj);
    QFile         saveFile(filepath);
    if(!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for writing: "
                 << saveFile.errorString();
        return;
    }

    QTextStream out(&saveFile);
    out << doc.toJson(QJsonDocument::Indented);
    saveFile.close();
}

QVector<Config::TranslatorParam> Config::translatorParams()
{
    QVector<Config::TranslatorParam> params;
    if(m_rootObj.isEmpty())
        return params;

    if(!m_rootObj.contains(KEY_TRANSLATOR_CONFIG)
       || !m_rootObj[KEY_TRANSLATOR_CONFIG].isArray())
    {
        qDebug() << "Config file does not contain 'translators' array.";
        return params;
    }

    QJsonArray arr = m_rootObj[KEY_TRANSLATOR_CONFIG].toArray();
    for(int i = 0; i < arr.size(); ++i)
    {
        Config::TranslatorParam param;
        auto                    obj = arr[i].toObject();
        param.id                    = obj["id"].toString();
        param.language              = obj["language"].toString();
        param.modelPath             = obj["model_path"].toString();
        param.translate             = obj["translate"].toBool(true);

        param.muteAmplitudeDurationMs =
            obj["mute_amplitude_duration_ms"].toInt(200);
        param.muteAmplitudeThreshold =
            obj["mute_amplitude_threshold"].toDouble(0.03);

        param.minAudioBufferSize = obj["min_audio_buffer_size"].toInt(32000);
        param.minNewSampleSize   = obj["min_new_sample_size"].toInt(6400);
        params.append(param);
    }
    return params;
}

QVector<Config::NetworkConfig> Config::networkConfigs()
{
    QVector<Config::NetworkConfig> configs;
    if(m_rootObj.isEmpty())
        return configs;

    if(!m_rootObj.contains(KEY_NETWORK_CONFIG)
       || !m_rootObj[KEY_NETWORK_CONFIG].isArray())
    {
        qDebug() << "Config file does not contain 'network_configs' array.";
        return configs;
    }

    QJsonArray arr = m_rootObj[KEY_NETWORK_CONFIG].toArray();
    for(int i = 0; i < arr.size(); ++i)
    {
        Config::NetworkConfig config;
        auto                  obj = arr[i].toObject();
        config.ip                 = obj["ip"].toString();
        config.port               = obj["port"].toInt();
        config.isEnable           = obj["isEnable"].toBool(true);
        configs.append(config);
    }
    return configs;
}