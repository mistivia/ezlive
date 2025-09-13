#ifndef TRANSCODE_TALKER_H_
#define TRANSCODE_TALKER_H_

#include "ringbuf.h"
#include "pthread.h"
#include <bits/pthreadtypes.h>

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t streaming_cond;
    RingBuffer *stream;
    bool quit;
} TranscodeTalker;

void TranscodeTalker_init(TranscodeTalker *self);

void* TranscodeTalker_main(void *vself);
void TranscodeTalker_new_stream(TranscodeTalker *self, RingBuffer *ringbuf);

#endif
