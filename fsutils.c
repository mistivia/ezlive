#include "fsutils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "s3_worker.h"

const char hextable[] = "0123456789abcdef";

void tmp_local_filename(const char *prefix, char *buf) {
    int prefix_len = strlen(prefix);
    memcpy(buf, prefix, prefix_len);
    buf = buf + prefix_len;
    size_t i;
    for (i = 0; i < 4; i++) {
        unsigned char r = rand() & 0xFF;
        buf[i * 2] = hextable[r >> 4];
        buf[i * 2 + 1] = hextable[r & 0xF];
    }
    buf[i*2] = '\0';
}

void tmp_ts_prefix(char *buf) {
    size_t i;
    for (i = 0; i < 4; i++) {
        unsigned char r = rand() & 0xFF;
        buf[i * 2] = hextable[r >> 4];
        buf[i * 2 + 1] = hextable[r & 0xF];
    }
    buf[i*2] = '\0';
}

void ts_filename(const char *prefix, int num, char *buf) {
    snprintf(buf, 256, "s%s%04d.ts", prefix, num);
}

void upload_file(const char *local, const char *remote) {
    s3_worker_push(s3_upload_task(local, remote));
}

void remove_remote(const char *remote) {
    s3_worker_push(s3_delete_task(remote));
}
