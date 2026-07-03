#ifndef GLOBAL_H
#define GLOBAL_H

static constexpr const char *TIMESTAMP_FMT = "yyyy-MM-dd hh:mm:ss";

static constexpr const char *CONFIG_FILE       = "./config.json";
static constexpr const char *MODEL_CONFIG_FILE = "./models.json";

static constexpr const char *KEY_NETWORK_CONFIG    = "networks";
static constexpr const char *KEY_TRANSLATOR_CONFIG = "audio translators";

static constexpr const char *PIPELINE_LOCAL  = "local";
static constexpr const char *PIPELINE_REMOTE = "remote";
static constexpr const char *PIPELINE_HYBRID = "hybrid";

#endif // GLOBAL_H