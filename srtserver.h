#ifndef SRTSERVER_H_
#define SRTSERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    void (*on_start)(void *ctx);
    void (*on_stop)(void *ctx);
    void (*on_data)(void* ctx, char *buf, size_t size);
} SrtCallbacks;

void start_srt_server(SrtCallbacks cbs, void *ctx);

#ifdef __cplusplus
}
#endif


#endif