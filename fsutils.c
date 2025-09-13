#include "fsutils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tmp_local_filename(const char *prefix, char *buf) {
    static const char hex[] = "0123456789abcdef";
    int prefix_len = strlen(prefix);
    memcpy(buf, prefix, prefix_len);
    buf = buf + prefix_len;
    size_t i;
    for (i = 0; i < 4; i++) {
        unsigned char r = rand() & 0xFF;
        buf[i * 2] = hex[r >> 4];
        buf[i * 2 + 1] = hex[r & 0xF];
    }
    buf[i*2] = '\0';
}

void tmp_ts_prefix(char *buf) {
    static const char hex[] = "0123456789abcdef";
    size_t i;
    for (i = 0; i < 4; i++) {
        unsigned char r = rand() & 0xFF;
        buf[i * 2] = hex[r >> 4];
        buf[i * 2 + 1] = hex[r & 0xF];
    }
    buf[i*2] = '\0';
}

void ts_filename(const char *prefix, int num, char *buf) {
    snprintf(buf, 256, "s%s%04d.ts", prefix, num);
}

void upload_file(const char *local, const char *remote) {
    // TODO
}

void remove_remote(const char *remote) {
    // TODO
}

char ** list_file() {
    // TODO
    return NULL;
}