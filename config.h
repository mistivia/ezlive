#ifndef EZLIVE_CONFIG_H_
#define EZLIVE_CONFIG_H_

typedef struct {
    const char *listening_addr;
    int port;
    const char *bucket;
    const char *endpoint;
    const char *dir;
    const char *access_key;
    const char *secret_key;
    const char *web_endpoint;
} EZLiveConfig;

extern EZLiveConfig *ezlive_config;

void EZLiveConfig_init(EZLiveConfig *self);
void EZLiveConfig_load(EZLiveConfig *self, const char *filename);

#endif