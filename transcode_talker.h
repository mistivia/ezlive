#ifndef TRANSCODE_TALKER_H_
#define TRANSCODE_TALKER_H_

#include "ringbuf.h"
#include "pthread.h"
#include <bits/pthreadtypes.h>

typedef struct {
    char *files[15];
    double times[15];
    int len;
} HlsList;

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t streaming_cond;
    RingBuffer *stream;
    bool quit;
    HlsList lst;
    time_t last_updated;
} TranscodeTalker;

void TranscodeTalker_init(TranscodeTalker *self);

void* TranscodeTalker_main(void *vself);
void TranscodeTalker_new_stream(TranscodeTalker *self, RingBuffer *ringbuf);

#endif
