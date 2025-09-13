#ifndef RTMPSERVER_H_
#define RTMPSERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    void (*on_start)(void *ctx);
    void (*on_stop)(void *ctx);
    void (*on_video)(void* ctx, int64_t timestamp, char *buf, size_t size);
    void (*on_audio)(void* ctx, int64_t timestamp, char *buf, size_t size);
} RtmpCallbacks;

void start_rtmpserver(RtmpCallbacks cbs, void *ctx);

#ifdef __cplusplus
}
#endif


#endif