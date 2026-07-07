#include "Config.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileDialog>
#include <QSaveFile>
#include <QDir>

#include "Global.h"

Config::Config(QObject *parent)
    : QObject(parent)
{
    load(QDir::current().filePath(CONFIG_FILE));
    loadModel(QDir::current().filePath(MODEL_CONFIG_FILE));
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

void Config::loadModel(const QString &filepath)
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
            return;
        }
        readFile.close();
    } else
    {
        qDebug() << "Failed to open model config file for reading: "
                 << readFile.errorString();
        return;
    }

    m_modelArr = doc.array();
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

void Config::saveModel(const QString &filepath)
{
    QJsonDocument doc(m_modelArr);
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

bool Config::isCoreRun()
{
    return m_rootObj.value("core_run").toBool(true);
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
        param.printTimestamps    = obj["print_timestamps"].toBool(false);
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
        param.maxAudioBufferSize = obj["max_audio_buffer_size"].toInt(32000);
        param.minNewSampleSize   = obj["min_new_sample_size"].toInt(6400);

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
    for(const auto &config : modelConfigs())
    {
        if(config.id == id)
            return config;
    }

    Config::ModelConfig conf;
    conf.id = "";
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

        // model params
        config.mainGPU          = obj["main_gpu"].toString();
        config.vocabOnly        = obj["vocab_only"].toBool(false);
        config.useMMap          = obj["use_mmap"].toBool(true);
        config.useDirectIO      = obj["use_direct_io"].toBool(false);
        config.useMLock         = obj["use_mlock"].toBool(false);
        config.checkTensors     = obj["check_tensors"].toBool(false);
        config.useExtraBufTypes = obj["use_extra_buf_types"].toBool(false);
        config.noHost           = obj["no_host"].toBool(false);
        config.noAlloc          = obj["no_alloc"].toBool(false);

        // context params
        config.nCtx          = obj["n_ctx"].toInt();
        config.nBatch        = obj["n_batch"].toInt();
        config.nUbatch       = obj["n_ubatch"].toInt();
        config.nSeqMax       = obj["n_seq_max"].toInt();
        config.nThreads      = obj["n_threads"].toInt();
        config.nThreadsBatch = obj["n_threads_batch"].toInt();

        config.ropeFreqBase   = obj["rope_freq_base"].toDouble();
        config.ropeFreqScale  = obj["rope_freq_scale"].toDouble();
        config.yarnExtFactor  = obj["yarn_ext_factor"].toDouble();
        config.yarnAttnFactor = obj["yarn_attn_factor"].toDouble();
        config.yarnBetaFast   = obj["yarn_beta_fast"].toDouble();
        config.yarnBetaSlow   = obj["yarn_beta_slow"].toDouble();
        config.yarnOrigCtx    = obj["yarn_orig_ctx"].toInt();
        config.defragThold    = obj["defrag_thold"].toDouble();

        config.embeddings = obj["embeddings"].toBool(false);
        config.offloadKQV = obj["offload_kqv"].toBool(false);
        config.noPerf     = obj["no_perf"].toBool(false);
        config.opOffload  = obj["op_offload"].toBool(false);
        config.swaFull    = obj["swa_full"].toBool(false);
        config.kvUnified  = obj["kv_unified"].toBool(false);

        // sampling parameters
        config.temperature       = obj["temperature"].toDouble();
        config.topP              = obj["top_p"].toDouble();
        config.topPMinKeep       = obj["top_p_min_keep"].toInt();
        config.topK              = obj["top_k"].toDouble();
        config.reputationPenalty = obj["reputation_penalty"].toDouble();
        config.minP              = obj["min_p"].toDouble();
        config.minPMinKeep       = obj["min_p_min_keep"].toInt();

        // control parameters
        config.ctxWindowSize = obj["ctx_window_size"].toInt();
        config.stopWords     = obj["stop_words"].toString();

        // prompt
        config.prompt = obj["prompt"].toString();

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

        // model params
        obj["main_gpu"]            = config.mainGPU;
        obj["vocab_only"]          = config.vocabOnly;
        obj["use_mmap"]            = config.useMMap;
        obj["use_direct_io"]       = config.useDirectIO;
        obj["use_mlock"]           = config.useMLock;
        obj["check_tensors"]       = config.checkTensors;
        obj["use_extra_buf_types"] = config.useExtraBufTypes;
        obj["no_host"]             = config.noHost;
        obj["no_alloc"]            = config.noAlloc;

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
        obj["temperature"]    = QString::number(config.temperature, 'f', 1);
        obj["top_p"]          = QString::number(config.topP, 'f', 1);
        obj["top_p_min_keep"] = QString::number(config.topPMinKeep);
        obj["top_k"]          = QString::number(config.topK, 'f', 1);
        obj["reputation_penalty"] =
            QString::number(config.reputationPenalty, 'f', 1);
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