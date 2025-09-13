#include <stdio.h>

#include "rtmpserver.h"
#include "ringbuf.h"
#include "transcode_talker.h"

typedef struct {
    RingBuffer *ringbuf;
    TranscodeTalker transcode_talker;
} MainCtx;

void on_rtmp_start(void *ctx) {
    MainCtx *main_ctx = ctx;
    main_ctx->ringbuf = malloc(sizeof(RingBuffer));
    RingBuffer_init(main_ctx->ringbuf, 4096);
    RingBuffer *rb = main_ctx->ringbuf;
    TranscodeTalker_new_stream(&main_ctx->transcode_talker, rb);

    RingBuffer_write_char(rb, 'F');
    RingBuffer_write_char(rb, 'L');
    RingBuffer_write_char(rb, 'V');
    RingBuffer_write_char(rb, 1);
    RingBuffer_write_char(rb, 5);
    RingBuffer_write_word32be(rb, 9);
    RingBuffer_write_word32be(rb, 0);
}

void on_rtmp_stop(void *ctx) {
    MainCtx *main_ctx = ctx;
    RingBuffer_end(main_ctx->ringbuf);
}

void on_rtmp_video(void *ctx, int64_t timestamp, char *buf, size_t size) {
    MainCtx *main_ctx = ctx;
    RingBuffer *rb = main_ctx->ringbuf;
    RingBuffer_write_char(rb, 9);
    RingBuffer_write_word24be(rb, size);
    RingBuffer_write_word24be(rb, timestamp);
    RingBuffer_write_char(rb, timestamp >> 24);
    RingBuffer_write_word24be(rb, 0);
    RingBuffer_write(rb, (const uint8_t *)buf, size);
    RingBuffer_write_word32be(rb, size + 11);
}

void on_rtmp_audio(void *ctx, int64_t timestamp, char *buf, size_t size) {
    MainCtx *main_ctx = ctx;
    RingBuffer *rb = main_ctx->ringbuf;
    RingBuffer_write_char(rb, 8);
    RingBuffer_write_word24be(rb, size);
    RingBuffer_write_word24be(rb, timestamp);
    RingBuffer_write_char(rb, timestamp >> 24);
    RingBuffer_write_word24be(rb, 0);
    RingBuffer_write(rb, (const uint8_t *)buf, size);
    RingBuffer_write_word32be(rb, size + 11);
}

int main() {
    MainCtx main_ctx;
    RtmpCallbacks rtmp_cbs = {
        .on_audio = &on_rtmp_audio,
        .on_video = &on_rtmp_video,
        .on_start = &on_rtmp_start,
        .on_stop = &on_rtmp_stop,
    };

    TranscodeTalker_init(&main_ctx.transcode_talker);
    pthread_t transmux_thread;
    pthread_create(&transmux_thread, NULL, &TranscodeTalker_main, &main_ctx.transcode_talker);

    start_rtmpserver(rtmp_cbs, &main_ctx);
    return 0;
}