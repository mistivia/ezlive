#include "ezlive_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

EZLiveConfig *ezlive_config;

static char *trim(char *s) {
    char *end;
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

void EZLiveConfig_init(EZLiveConfig *self) {
    if (!self) return;
    self->listening_addr = strdup("127.0.0.1");
    self->listening_port = 1935;
    self->bucket = NULL;
    self->endpoint = NULL;
    self->s3_path = strdup("ezlive/");
    self->access_key = NULL;
    self->secret_key = NULL;
    self->region = strdup("auto");
    self->key = strdup("");
}

static void set_field(const char **field, const char *value) {
    if (*field) {
        free((void *)*field);
    }
    *field = strdup(value);
}

void EZLiveConfig_load(EZLiveConfig *self, const char *filename) {
    if (!self || !filename) return;

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        char *hash = strchr(line, '#');
        if (hash) *hash = 0;

        char *trimmed = trim(line);
        if (*trimmed == 0) continue;

        char *eq = strchr(trimmed, '=');
        if (!eq) continue;

        *eq = 0;
        char *key = trim(trimmed);
        char *val = trim(eq + 1);

        if (strcmp(key, "listening_addr") == 0) {
            set_field(&self->listening_addr, val);
        } else if (strcmp(key, "listening_port") == 0) {
            self->listening_port = atoi(val);
        } else if (strcmp(key, "bucket") == 0) {
            set_field(&self->bucket, val);
        } else if (strcmp(key, "endpoint") == 0) {
            set_field(&self->endpoint, val);
        } else if (strcmp(key, "s3_path") == 0) {
            set_field(&self->s3_path, val);
        } else if (strcmp(key, "access_key") == 0) {
            set_field(&self->access_key, val);
        } else if (strcmp(key, "secret_key") == 0) {
            set_field(&self->secret_key, val);
        } else if (strcmp(key, "region") == 0) {
            set_field(&self->region, val);
        } else if (strcmp(key, "key") == 0) {
            set_field(&self->key, val);
        }
    }

    fclose(fp);
}

int EZLiveConfig_validate(EZLiveConfig *self) {
    if (!self) return -1;
    if (!self->listening_addr || strlen(self->listening_addr) == 0) return -2;
    if (!self->bucket || strlen(self->bucket) == 0) return -3;
    if (!self->endpoint || strlen(self->endpoint) == 0) return -4;
    if (!self->s3_path || strlen(self->s3_path) < 2) return -5;
    if (self->s3_path[strlen(self->s3_path) - 1] != '/') {
        fprintf(stderr, "invalid s3 path. path should end with '\'.\n");
        return -10;
    }
    if (!self->access_key || strlen(self->access_key) == 0) return -6;
    if (!self->secret_key || strlen(self->secret_key) == 0) return -7;
    if (self->listening_port <= 0 || self->listening_port > 65535) return -8;
    if (!self->region || strlen(self->region) == 0) return -9;
    return 0;
}