#include "Config.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileDialog>
#include <QSaveFile>
#include <QDir>
#include <QMessageBox>
#include <QCoreApplication>

#include "Global.h"

Config::Config(QObject *parent)
    : QObject(parent)
{
    if(!load(Config::getConfigFilePath()))
    {
        QMessageBox::critical(nullptr,
                              tr("Load Config File Failed"),
                              tr("Failed to load config file: %1")
                                  .arg(Config::getConfigFilePath()));
        return;
    }

    loadModel(Config::getModelConfigFilePath());
    loadMemory(Config::getMemoryConfigFilePath());
}

Config::~Config()
{
}

Config &Config::Instance()
{
    static Config instance;
    return instance;
}

QString Config::getConfigFilePath()
{
    return QCoreApplication::applicationDirPath() + CONFIG_FILE;
}

QString Config::getModelConfigFilePath()
{
    return QCoreApplication::applicationDirPath() + MODEL_CONFIG_FILE;
}

QString Config::getMemoryConfigFilePath()
{
    return QCoreApplication::applicationDirPath() + MEMORY_CONFIG_FILE;
}

bool Config::load(const QString &filepath)
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
            return false;
        }
        readFile.close();
    } else
    {
        qDebug() << "Failed to open config file for reading: "
                 << readFile.errorString();
        return false;
    }

    m_rootObj = doc.object();

    // build supported doc types
    if(m_rootObj.contains("supported_doc_types"))
    {
        auto arr = m_rootObj["supported_doc_types"].toArray();
        for(auto typ : arr)
            m_supportedDocTypes.insert(typ.toString());
    }
    return true;
}

bool Config::loadModel(const QString &filepath)
{
    QFile         readFile(filepath);
    QJsonDocument doc;
    if(readFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray      data = readFile.readAll();
        QJsonParseError parseError;
        doc = QJsonDocument::fromJson(data, &parseError);
        if(parseError.error != QJsonParseError::NoError || !doc.isArray())
        {
            qDebug() << "Failed to parse JSON from model config file: "
                     << filepath
                     << ", with parse error:" << parseError.errorString();
            return false;
        }
        readFile.close();
    } else
    {
        qDebug() << "Failed to open model config file for reading: "
                 << readFile.errorString();
        return false;
    }

    m_modelArr = doc.array();
    return true;
}

bool Config::loadMemory(const QString &filepath)
{
    QFile         readFile(filepath);
    QJsonDocument doc;
    if(readFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray      data = readFile.readAll();
        QJsonParseError parseError;
        doc = QJsonDocument::fromJson(data, &parseError);
        if(parseError.error != QJsonParseError::NoError || !doc.isArray())
        {
            qDebug() << "Failed to parse JSON from memory config file: "
                     << filepath
                     << ", with parse error:" << parseError.errorString();
            return false;
        }
        readFile.close();
    } else
    {
        qDebug() << "Failed to open memory config file for reading: "
                 << readFile.errorString();
        return false;
    }

    m_memoryArr = doc.array();
    return true;
}

bool Config::save(const QString &filepath)
{
    QJsonDocument doc(m_rootObj);
    QFile         saveFile(filepath);
    if(!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for writing: "
                 << saveFile.errorString();
        return false;
    }

    QTextStream out(&saveFile);
    out << doc.toJson(QJsonDocument::Indented);
    saveFile.close();
    return true;
}

bool Config::saveModel(const QString &filepath)
{
    QJsonDocument doc(m_modelArr);
    QFile         saveFile(filepath);
    if(!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for writing: "
                 << saveFile.errorString();
        return false;
    }

    QTextStream out(&saveFile);
    out << doc.toJson(QJsonDocument::Indented);
    saveFile.close();
    return true;
}

bool Config::saveMemory(const QString &filepath)
{
    QJsonDocument doc(m_memoryArr);
    QFile         saveFile(filepath);
    if(!saveFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open file for writing: "
                 << saveFile.errorString();
        return false;
    }

    QTextStream out(&saveFile);
    out << doc.toJson(QJsonDocument::Indented);
    saveFile.close();
    return true;
}

bool Config::isCoreRun()
{
    return m_rootObj.value("core_run").toBool(true);
}

QSet<QString> Config::getSupportedDocTypes()
{
    return m_supportedDocTypes;
}

QVector<QString> Config::getAppUpgradeUrls()
{
    QVector<QString> urls;
    if(m_rootObj.contains("app_upgrade_urls")
       && m_rootObj["app_upgrade_urls"].isArray())
    {
        QJsonArray arr = m_rootObj["app_upgrade_urls"].toArray();
        for(const auto &url : arr)
            urls.append(url.toString());

        return urls;
    }

    return urls;
}

QVector<QString> Config::getPluginUploadUrls()
{
    QVector<QString> urls;
    if(m_rootObj.contains("plugin_upload_urls")
       && m_rootObj["plugin_upload_urls"].isArray())
    {
        QJsonArray arr = m_rootObj["plugin_upload_urls"].toArray();
        for(const auto &url : arr)
            urls.append(url.toString());

        return urls;
    }

    return urls;
}

QVector<Bus::AudioParam> Config::getBusAudioParams()
{
    QVector<Bus::AudioParam> params;
    for(const auto &param : audioTranslatorParams())
    {
        Bus::AudioParam audioParam;
        audioParam.translatorId       = param.id;
        audioParam.minNewSampleSize   = param.minNewSampleSize;
        audioParam.minAudioBufferSize = param.minAudioBufferSize;
        audioParam.maxAudioBufferSize = param.maxAudioBufferSize;
        params.append(audioParam);
    }
    return params;
}

Config::TranslatorParam Config::getAudioTranslatorParamById(const QString &id)
{
    // TODO optimise performance later
    for(const auto &param : audioTranslatorParams())
    {
        if(param.id == id)
            return param;
    }

    Config::TranslatorParam param;
    param.id = "";
    return param;
}

QVector<Config::TranslatorParam> Config::audioTranslatorParams()
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

        // full param
        param.nThreads    = obj["n_threads"].toInt();
        param.nMaxTextCtx = obj["n_max_text_ctx"].toInt();
        param.offsetMs    = obj["offset_ms"].toInt();
        param.durationMs  = obj["duration_ms"].toInt();

        param.translate      = obj["translate"].toBool(false);
        param.detectLanguage = obj["detect_language"].toBool(true);
        param.language       = obj["language"].toString("auto");

        param.noCtx              = obj["no_ctx"].toBool(true);
        param.noTimestamps       = obj["no_timestamps"].toBool(false);
        param.singleSegment      = obj["single_segment"].toBool(false);
        param.printSpecial       = obj["print_special"].toBool(false);
        param.printProgress      = obj["print_progress"].toBool(false);
        param.printRealtime      = obj["print_realtime"].toBool(false);
        param.printTimestamps    = obj["print_timestamps"].toBool(false);
        param.carryInitialPrompt = obj["carry_initial_prompt"].toBool(false);
        param.initialPrompt      = obj["initial_prompt"].toString("");
        param.suppressRegex      = obj["suppress_regex"].toString("");
        param.suppressBlank      = obj["suppress_blank"].toBool(true);
        param.suppressNst        = obj["suppress_nst"].toBool(false);

        param.temperature    = obj["temperature"].toString().toDouble();
        param.temperatureInc = obj["temperature_inc"].toString().toDouble();

        param.maxInitialTs  = obj["max_initial_ts"].toString().toDouble();
        param.lengthPenalty = obj["length_penalty"].toString().toDouble();
        param.entropyThold  = obj["entropy_thold"].toString().toDouble();
        param.logprobThold  = obj["logprob_thold"].toString().toDouble();
        param.noSpeechThold = obj["no_speech_thold"].toString().toDouble();

        // mute control
        param.minAudioBufferSize = obj["min_audio_buffer_size"].toInt();
        param.maxAudioBufferSize = obj["max_audio_buffer_size"].toInt();
        param.minNewSampleSize   = obj["min_new_sample_size"].toInt();

        params.append(param);
    }
    return params;
}

void Config::setAudioTranslatorParams(QVector<Config::TranslatorParam> &params)
{
    QJsonArray arr;
    for(auto param : params)
    {
        QJsonObject obj;
        obj["id"] = param.id;

        // full param
        obj["n_threads"]      = param.nThreads;
        obj["n_max_text_ctx"] = param.nMaxTextCtx;
        obj["offset_ms"]      = param.offsetMs;
        obj["duration_ms"]    = param.durationMs;

        obj["translate"]       = param.translate;
        obj["detect_language"] = param.detectLanguage;
        obj["language"]        = param.language;

        obj["no_ctx"]               = param.noCtx;
        obj["no_timestamps"]        = param.noTimestamps;
        obj["single_segment"]       = param.singleSegment;
        obj["print_special"]        = param.printSpecial;
        obj["print_progress"]       = param.printProgress;
        obj["print_realtime"]       = param.printRealtime;
        obj["print_timestamps"]     = param.printTimestamps;
        obj["carry_initial_prompt"] = param.carryInitialPrompt;
        obj["initial_prompt"]       = param.initialPrompt;
        obj["suppress_regex"]       = param.suppressRegex;
        obj["suppress_blank"]       = param.suppressBlank;
        obj["suppress_nst"]         = param.suppressNst;

        obj["temperature"]     = QString::number(param.temperature, 'f', 1);
        obj["temperature_inc"] = QString::number(param.temperatureInc, 'f', 1);

        obj["max_initial_ts"]  = QString::number(param.maxInitialTs, 'f', 1);
        obj["length_penalty"]  = QString::number(param.lengthPenalty, 'f', 1);
        obj["entropy_thold"]   = QString::number(param.entropyThold, 'f', 1);
        obj["logprob_thold"]   = QString::number(param.logprobThold, 'f', 1);
        obj["no_speech_thold"] = QString::number(param.noSpeechThold, 'f', 1);

        // mute control
        obj["min_audio_buffer_size"] = param.minAudioBufferSize;
        obj["max_audio_buffer_size"] = param.maxAudioBufferSize;
        obj["min_new_sample_size"]   = param.minNewSampleSize;

        arr.append(obj);
    }

    m_rootObj[KEY_TRANSLATOR_CONFIG] = arr;
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

void Config::setNetworkConfigs(QVector<Config::NetworkConfig> &configs)
{
    QJsonArray arr;
    for(auto config : configs)
    {
        QJsonObject obj;
        obj["ip"]       = config.ip;
        obj["port"]     = config.port;
        obj["isEnable"] = config.isEnable;
        arr.append(obj);
    }
    m_rootObj[KEY_NETWORK_CONFIG] = arr;
}

Config::ModelConfig Config::getModelConfigById(const QString &id)
{
    Config::ModelConfig conf;
    conf.id = "";

    for(int i = 0; i < m_modelArr.size(); ++i)
    {
        auto obj = m_modelArr[i].toObject();
        if(obj["id"].toString() == id)
        {
            _convert(conf, obj);
            return conf;
        }
    }

    return conf;
}

QVector<Config::ModelConfig> Config::modelConfigs()
{
    QVector<Config::ModelConfig> configs;
    if(m_modelArr.isEmpty())
    {
        qDebug() << "Config file does not contain model config array.";
        return configs;
    }

    for(int i = 0; i < m_modelArr.size(); ++i)
    {
        Config::ModelConfig config;
        auto                obj = m_modelArr[i].toObject();
        _convert(config, obj);
        configs.append(config);
    }
    return configs;
}

void Config::setModelConfigs(QVector<Config::ModelConfig> &configs)
{
    m_modelArr = QJsonArray();
    for(auto config : configs)
    {
        QJsonObject obj;

        // base info
        obj["id"]        = config.id;
        obj["name"]      = config.name;
        obj["publisher"] = config.publisher;
        obj["timestamp"] = config.timestamp;
        obj["addr"]      = config.addr;
        obj["pipeline"]  = config.pipeline;
        obj["cost"]      = QString::number(config.cost, 'f', 1);
        obj["api_key"]   = config.apiKey;
        obj["hash"]      = config.hash;

        // context params
        obj["n_ctx"]           = config.nCtx;
        obj["n_batch"]         = config.nBatch;
        obj["n_ubatch"]        = config.nUbatch;
        obj["n_seq_max"]       = config.nSeqMax;
        obj["n_threads"]       = config.nThreads;
        obj["n_threads_batch"] = config.nThreadsBatch;

        obj["rope_freq_base"]  = QString::number(config.ropeFreqBase, 'f', 1);
        obj["rope_freq_scale"] = QString::number(config.ropeFreqScale, 'f', 1);
        obj["yarn_ext_factor"] = QString::number(config.yarnExtFactor, 'f', 1);
        obj["yarn_attn_factor"] =
            QString::number(config.yarnAttnFactor, 'f', 1);
        obj["yarn_beta_fast"] = QString::number(config.yarnBetaFast, 'f', 1);
        obj["yarn_beta_slow"] = QString::number(config.yarnBetaSlow, 'f', 1);
        obj["yarn_orig_ctx"]  = config.yarnOrigCtx;
        obj["defrag_thold"]   = QString::number(config.defragThold, 'f', 1);

        obj["embeddings"]  = config.embeddings;
        obj["offload_kqv"] = config.offloadKQV;
        obj["no_perf"]     = config.noPerf;
        obj["op_offload"]  = config.opOffload;
        obj["swa_full"]    = config.swaFull;
        obj["kv_unified"]  = config.kvUnified;

        // sampling parameters
        obj["penalty_last_n"]  = QString::number(config.penaltyLastN);
        obj["penalty_repeat"]  = QString::number(config.penaltyRepeat, 'f', 1);
        obj["penalty_freq"]    = QString::number(config.penaltyFreq, 'f', 1);
        obj["penalty_present"] = QString::number(config.penaltyPresent, 'f', 1);

        obj["temperature"]     = QString::number(config.temperature, 'f', 1);
        obj["temperature_ext"] = QString::number(config.temperatureExt, 'f', 1);
        obj["temperature_ext_delta"] =
            QString::number(config.temperatureExtDelta, 'f', 1);
        obj["temperature_ext_exponent"] =
            QString::number(config.temperatureExtExponent, 'f', 1);

        obj["seed"] = QString::number(config.seed);

        obj["top_p"]          = QString::number(config.topP, 'f', 1);
        obj["top_p_min_keep"] = QString::number(config.topPMinKeep);
        obj["top_k"]          = QString::number(config.topK);
        obj["min_p"]          = QString::number(config.minP, 'f', 1);
        obj["min_p_min_keep"] = QString::number(config.minPMinKeep);

        // control parameters
        obj["ctx_window_size"] = config.ctxWindowSize;
        obj["stop_words"]      = config.stopWords;

        // prompt
        obj["prompt"] = config.prompt;

        m_modelArr.append(obj);
    }
}

void Config::_convert(Config::ModelConfig &config, const QJsonObject &obj)
{
    // base info
    config.id        = obj["id"].toString();
    config.name      = obj["name"].toString();
    config.publisher = obj["publisher"].toString();
    config.timestamp = obj["timestamp"].toString();
    config.addr      = obj["addr"].toString();
    config.pipeline  = obj["pipeline"].toString();
    config.cost      = obj["cost"].toInt();
    config.apiKey    = obj["api_key"].toString();
    config.hash      = obj["hash"].toString();

    // context params
    config.nCtx          = obj["n_ctx"].toInt();
    config.nBatch        = obj["n_batch"].toInt();
    config.nUbatch       = obj["n_ubatch"].toInt();
    config.nSeqMax       = obj["n_seq_max"].toInt();
    config.nThreads      = obj["n_threads"].toInt();
    config.nThreadsBatch = obj["n_threads_batch"].toInt();

    config.ropeFreqBase   = obj["rope_freq_base"].toString().toDouble();
    config.ropeFreqScale  = obj["rope_freq_scale"].toString().toDouble();
    config.yarnExtFactor  = obj["yarn_ext_factor"].toString().toDouble();
    config.yarnAttnFactor = obj["yarn_attn_factor"].toString().toDouble();
    config.yarnBetaFast   = obj["yarn_beta_fast"].toString().toDouble();
    config.yarnBetaSlow   = obj["yarn_beta_slow"].toString().toDouble();
    config.yarnOrigCtx    = obj["yarn_orig_ctx"].toInt();
    config.defragThold    = obj["defrag_thold"].toString().toDouble();

    config.embeddings = obj["embeddings"].toBool(false);
    config.offloadKQV = obj["offload_kqv"].toBool(false);
    config.noPerf     = obj["no_perf"].toBool(false);
    config.opOffload  = obj["op_offload"].toBool(false);
    config.swaFull    = obj["swa_full"].toBool(false);
    config.kvUnified  = obj["kv_unified"].toBool(false);

    // sampling parameters
    config.penaltyLastN   = obj["penalty_last_n"].toInt();
    config.penaltyRepeat  = obj["penalty_repeat"].toString().toDouble();
    config.penaltyFreq    = obj["penalty_freq"].toString().toDouble();
    config.penaltyPresent = obj["penalty_present"].toString().toDouble();

    config.temperature    = obj["temperature"].toString().toDouble();
    config.temperatureExt = obj["temperature_ext"].toString().toDouble();
    config.temperatureExtDelta =
        obj["temperature_ext_delta"].toString().toDouble();
    config.temperatureExtExponent =
        obj["temperature_ext_exponent"].toString().toDouble();

    config.seed = obj["seed"].toInt();

    config.topP        = obj["top_p"].toString().toDouble();
    config.topPMinKeep = obj["top_p_min_keep"].toInt();
    config.topK        = obj["top_k"].toInt();
    config.minP        = obj["min_p"].toString().toDouble();
    config.minPMinKeep = obj["min_p_min_keep"].toInt();

    // control parameters
    config.ctxWindowSize = obj["ctx_window_size"].toInt();
    config.stopWords     = obj["stop_words"].toString();

    // prompt
    config.prompt = obj["prompt"].toString();
}

QString Config::getDefaultIndexPath()
{
    return m_rootObj.value("default_index_path").toString();
}

Config::MemoryConfig Config::getMemoryConfigById(const QString &id)
{
    Config::MemoryConfig conf;
    conf.id = "";
    if(m_memoryArr.isEmpty())
    {
        qDebug() << "Config file does not contain memory config array.";
        return conf;
    }

    for(int i = 0; i < m_memoryArr.size(); ++i)
    {
        auto obj = m_memoryArr[i].toObject();
        if(obj["id"].toString() != id)
            continue;

        conf.id             = obj["id"].toString();
        conf.indexFilePath  = obj["index_file_path"].toString();
        conf.metaFilePath   = obj["meta_file_path"].toString();
        conf.originFilePath = obj["origin_file_path"].toString();
        conf.dimension      = obj["dimension"].toInt();

        // chunk param
        conf.chunkSize         = obj["chunk_size"].toInt();
        conf.overlap           = obj["overlap"].toInt();
        conf.respectSentences  = obj["respect_sentences"].toBool();
        conf.respectParagraphs = obj["respect_paragraphs"].toBool();
        conf.encoding          = obj["encoding"].toString();
        return conf;
    }
    return conf;
}

QVector<Config::MemoryConfig> Config::memoryConfigs()
{
    QVector<Config::MemoryConfig> configs;
    if(m_memoryArr.isEmpty())
    {
        qDebug() << "Config file does not contain memory config array.";
        return configs;
    }

    for(int i = 0; i < m_memoryArr.size(); ++i)
    {
        Config::MemoryConfig config;
        auto                 obj = m_memoryArr[i].toObject();
        config.id                = obj["id"].toString();
        config.indexFilePath     = obj["index_file_path"].toString();
        config.metaFilePath      = obj["meta_file_path"].toString();
        config.originFilePath    = obj["origin_file_path"].toString();
        config.dimension         = obj["dimension"].toInt();

        // chunk param
        config.chunkSize         = obj["chunk_size"].toInt();
        config.overlap           = obj["overlap"].toInt();
        config.respectSentences  = obj["respect_sentences"].toBool();
        config.respectParagraphs = obj["respect_paragraphs"].toBool();
        config.encoding          = obj["encoding"].toString();
        configs.append(config);
    }
    return configs;
}

void Config::setMemoryConfigs(QVector<Config::MemoryConfig> &configs)
{
    m_memoryArr = QJsonArray();
    for(const auto &config : configs)
    {
        QJsonObject obj;
        obj["id"]               = config.id;
        obj["index_file_path"]  = config.indexFilePath;
        obj["meta_file_path"]   = config.metaFilePath;
        obj["origin_file_path"] = config.originFilePath;
        obj["dimension"]        = config.dimension;

        // chunk param
        obj["chunk_size"]         = config.chunkSize;
        obj["overlap"]            = config.overlap;
        obj["respect_sentences"]  = config.respectSentences;
        obj["respect_paragraphs"] = config.respectParagraphs;
        obj["encoding"]           = config.encoding;
        m_memoryArr.append(obj);
    }
}