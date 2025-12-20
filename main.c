#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "ezlive_config.h"
#include "srtserver.h"
#include "ringbuf.h"
#include "transcode_talker.h"
#include "s3_worker.h"

typedef struct {
    RingBuffer *ringbuf;
    TranscodeTalker transcode_talker;
} MainCtx;

void on_srt_start(void *ctx) {
    MainCtx *main_ctx = ctx;
    main_ctx->ringbuf = malloc(sizeof(RingBuffer));
    RingBuffer_init(main_ctx->ringbuf, 4096);
    RingBuffer *rb = main_ctx->ringbuf;
    TranscodeTalker_new_stream(&main_ctx->transcode_talker, rb);
}

void on_srt_stop(void *ctx) {
    MainCtx *main_ctx = ctx;
    RingBuffer_end(main_ctx->ringbuf);
}

void on_srt_data(void *ctx, char *buf, size_t size) {
    MainCtx *main_ctx = ctx;
    RingBuffer *rb = main_ctx->ringbuf;
    RingBuffer_write(rb, (const uint8_t*)buf, size);
}

void cleantmpfile() {
#ifdef _WIN32
    system("del tmp*");
#else
    system("rm /tmp/ezlive*");
#endif
}

int main(int argc, char **argv) {
    cleantmpfile();
    ezlive_config = malloc(sizeof(EZLiveConfig));
    EZLiveConfig_init(ezlive_config);
    bool succ;
    if (argc == 1) {
        succ = EZLiveConfig_load(ezlive_config, "./config");
#if defined(_WIN32)
        if (!succ) {
            succ = EZLiveConfig_load(ezlive_config, "./config.txt");
        }
#endif
        if (!succ) {
            fprintf(stderr, "Failed to load config.\n");
            return -1;
        }
    } else if (argc == 2) {
        if (!EZLiveConfig_load(ezlive_config, argv[1])) {
            fprintf(stderr, "Failed to load config.\n");
            return -1;
        }
    } else {
        fprintf(stderr, "wrong args.\n");
        exit(-1);
    }
    int ret;
    if ((ret = EZLiveConfig_validate(ezlive_config)) < 0) {
        fprintf(stderr, "ezlive config error: %d.\n", ret);
        exit(-1);
    }
    srand((unsigned) time(NULL));
    MainCtx main_ctx;
    SrtCallbacks srt_cbs = {
        .on_data = &on_srt_data,
        .on_stop = &on_srt_stop,
        .on_start = &on_srt_start,
    };

    TranscodeTalker_init(&main_ctx.transcode_talker);
    s3_worker_init();
    s3_worker_push(s3_clear_task());

    pthread_t transmux_thread;
    pthread_create(&transmux_thread, NULL, &TranscodeTalker_main, &main_ctx.transcode_talker);
    pthread_t s3worker_thread;
    pthread_create(&s3worker_thread, NULL, &s3_worker_main, NULL);

    start_srt_server(srt_cbs, &main_ctx);
    return 0;
}