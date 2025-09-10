#include <stdio.h>

#include "rtmpserver.h"
#include "fileutils.h"

void on_rtmp_start(void *ctx) {
    *(FILE**)ctx = fopen("test.flv", "wb");
    FILE *fp = *(FILE**)ctx;
    fwrite_char(fp, 'F');
    fwrite_char(fp, 'L');
    fwrite_char(fp, 'V');
    fwrite_char(fp, 1);
    fwrite_char(fp, 5);
    fwrite_word32be(fp, 9);
    fwrite_word32be(fp, 0);
}

void on_rtmp_stop(void *ctx) {
    FILE *fp = *(FILE**)ctx;
    fclose(fp);
    exit(0);
}

void on_rtmp_video(void *ctx, int64_t timestamp, char *buf, size_t size) {
    FILE *fp = *(FILE**)ctx;
    fwrite_char(fp, 9);
    fwrite_word24be(fp, size);
    fwrite_word24be(fp, timestamp);
    fwrite_char(fp, timestamp >> 24);
    fwrite_word24be(fp, 0);
    fwrite(buf, 1, size, fp);
    fwrite_word32be(fp, size + 11);
}

void on_rtmp_audio(void *ctx, int64_t timestamp, char *buf, size_t size) {
    FILE *fp = *(FILE**)ctx;
    fwrite_char(fp, 8);
    fwrite_word24be(fp, size);
    fwrite_word24be(fp, timestamp);
    fwrite_char(fp, timestamp >> 24);
    fwrite_word24be(fp, 0);
    fwrite(buf, 1, size, fp);
    fwrite_word32be(fp, size + 11);
}

int main() {
    RtmpCallbacks rtmp_cbs = {
        .on_audio = &on_rtmp_audio,
        .on_video = &on_rtmp_video,
        .on_start = &on_rtmp_start,
        .on_stop = &on_rtmp_stop,
    };
    FILE* fp = NULL;
    start_rtmpserver(rtmp_cbs, &fp);
    return 0;
}