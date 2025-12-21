#ifndef EZLIVE_CONFIG_H_
#define EZLIVE_CONFIG_H_

#include <stdbool.h>

typedef struct {
    const char *listening_addr;
    int listening_port;
    const char *bucket;
    const char *endpoint;
    const char *s3_path;
    const char *access_key;
    const char *secret_key;
    const char *region;
    const char *key;
} EZLiveConfig;

extern EZLiveConfig *ezlive_config;

void EZLiveConfig_init(EZLiveConfig *self);
bool EZLiveConfig_load(EZLiveConfig *self, const char *filename);
int EZLiveConfig_validate(EZLiveConfig *self);

#endif