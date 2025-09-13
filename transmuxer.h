#ifndef TRANSMUXER_H_
#define TRANSMUXER_H_

#include "ringbuf.h"
#include "pthread.h"
#include <bits/pthreadtypes.h>

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t streaming_cond;
    RingBuffer *stream;
    bool quit;
} Transmuxer;

void Transmuxer_init(Transmuxer *self);

void* Transmuxer_main(void *vself);
void Transmuxer_new_stream(Transmuxer *self, RingBuffer *ringbuf);

#endif
