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
        param.modelPath             = obj["model_path"].toString();

        // ctx param
        param.useGPU         = obj["use_gpu"].toBool(false);
        param.gpuDevice      = obj["gpu_device"].toInt(0);
        param.flashAttention = obj["flash_attention"].toBool(true);

        // full param
        param.nThreads    = obj["n_threads"].toInt(4);
        param.nMaxTextCtx = obj["n_max_text_ctx"].toInt(16384);
        param.offsetMs    = obj["offset_ms"].toInt(0);
        param.durationMs  = obj["duration_ms"].toInt(0);

        param.translate      = obj["translate"].toBool(false);
        param.detectLanguage = obj["detect_language"].toBool(true);
        param.language       = obj["language"].toString("auto");

        param.noCtx              = obj["no_ctx"].toBool(true);
        param.noTimestamps       = obj["no_timestamps"].toBool(false);
        param.singleSegment      = obj["single_segment"].toBool(false);
        param.printSpecial       = obj["print_special"].toBool(false);
        param.printProgress      = obj["print_progress"].toBool(false);
        param.printRealtime      = obj["print_realtime"].toBool(false);
        param.carryInitialPrompt = obj["carry_initial_prompt"].toBool(false);
        param.initialPrompt      = obj["initial_prompt"].toString("");
        param.suppressRegex      = obj["suppress_regex"].toString("");
        param.suppressBlank      = obj["suppress_blank"].toBool(true);
        param.suppressNst        = obj["suppress_nst"].toBool(false);

        param.temperature    = obj["temperature"].toDouble(0.0);
        param.temperatureInc = obj["temperature_inc"].toDouble(0.2);

        param.maxInitialTs  = obj["max_initial_ts"].toDouble(1.0);
        param.lengthPenalty = obj["length_penalty"].toDouble(-1.0);
        param.entropyThold  = obj["entropy_thold"].toDouble(2.4);
        param.logprobThold  = obj["logprob_thold"].toDouble(-1.0);
        param.noSpeechThold = obj["no_speech_thold"].toDouble(0.6);

        // mute control
        param.minAudioBufferSize = obj["min_audio_buffer_size"].toInt(32000);
        param.minNewSampleSize   = obj["min_new_sample_size"].toInt(6400);
        param.muteAmplitudeDurationMs =
            obj["mute_amplitude_duration_ms"].toInt(200);
        param.muteAmplitudeThreshold =
            obj["mute_amplitude_threshold"].toDouble(0.03);
        param.keepLastAudioBufferMs =
            obj["keep_last_audio_buffer_ms"].toInt(1000);

        // sleep and wake up control
        param.sleepTimeoutMs      = obj["sleep_timeout_ms"].toInt(3000);
        param.wakeupThreshold     = obj["wakeup_threshold"].toDouble(0.01);
        param.maxSameContentCount = obj["max_same_content_count"].toInt(3);

        // noise control
        param.filtRegex = obj["filt_regex"].toString();
        param.noiseWords.clear();
        if(obj.contains("noise_words") && obj["noise_words"].isArray())
        {
            QJsonArray noiseArr = obj["noise_words"].toArray();
            for(int j = 0; j < noiseArr.size(); ++j)
                param.noiseWords.append(noiseArr[j].toString());
        }
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