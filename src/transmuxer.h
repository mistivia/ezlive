#ifndef TRANSCODE_TALKER_H_
#define TRANSCODE_TALKER_H_

#include "ringbuf.h"
#include "src/hls_list.h"
#include "pthread.h"

namespace ezlive {

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t streaming_cond;
    ring_buffer *stream;
    bool quit;
    hls_list m_lst;
    time_t last_updated;
} TranscodeTalker;

void TranscodeTalker_init(TranscodeTalker *self);

void* TranscodeTalker_main(void *vself);
void TranscodeTalker_new_stream(TranscodeTalker *self, ring_buffer *ringbuf);

}

#endif
