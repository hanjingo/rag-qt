#ifndef GLOBAL_H
#define GLOBAL_H

#ifndef RAG_QT_VERSION
#define RAG_QT_VERSION "0.0.1"
#endif

static constexpr const char *TIMESTAMP_FMT = "yyyy-MM-dd hh:mm:ss";
static constexpr const char *DATE_FMT      = "yyyy-MM-dd";

static constexpr const char *CONFIG_FILE = "configs/config.json";
#ifdef Q_OS_WIN
static constexpr const char *RAG_CORE = "rag-core.exe";
#else
static constexpr const char *RAG_CORE = "rag-core";
#endif
static constexpr const char *RAG_CORE_CONFIG = "configs/core.ini";

static constexpr const char *PIPELINE_LOCAL  = "local";
static constexpr const char *PIPELINE_REMOTE = "remote";
static constexpr const char *PIPELINE_HYBRID = "hybrid";

static constexpr const char *LANG_ZH_CN = "zh_CN";
static constexpr const char *LANG_EN_UK = "en_UK";
static constexpr const char *LANG_DE_DE = "de_DE";

static constexpr const char *KEY_CORE_CONFIG       = "core_config";
static constexpr const char *KEY_PLUGIN_PATH       = "plugin_path";
static constexpr const char *KEY_MODEL_CONFIG      = "model_config";
static constexpr const char *KEY_MEMORY_CONFIG     = "memory_config";
static constexpr const char *KEY_ASR_CONFIG        = "asr_config";
static constexpr const char *KEY_NETWORK_CONFIG    = "networks";
static constexpr const char *KEY_TRANSLATOR_CONFIG = "audio translators";

static constexpr const char *TOPIC_SEPARATOR  = "|";
static constexpr const char *TOPIC_RAG_CORE   = "topic-rag-core";
static constexpr const char *TOPIC_PLUGIN_PUB = "topic-plugin-pub";

#endif // GLOBAL_H