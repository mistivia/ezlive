#ifndef RINGBUF_H_
#define RINGBUF_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct {
    uint8_t *buffer;
    size_t head;
    size_t tail;
    size_t max;
    bool full_flag;
    bool finished_flag;

    pthread_mutex_t mutex;
    pthread_cond_t not_empty_cond;
    pthread_cond_t not_full_cond;
} RingBuffer;

void RingBuffer_init(RingBuffer *self, size_t size);

void RingBuffer_destroy(RingBuffer *self);

size_t RingBuffer_size(RingBuffer *self);

size_t RingBuffer_space(RingBuffer *self);

size_t RingBuffer_write(RingBuffer *self, const uint8_t *data, size_t len);

bool RingBuffer_write_word16le(RingBuffer *self, uint16_t x);
bool RingBuffer_write_word24le(RingBuffer *self, uint32_t x);
bool RingBuffer_write_word32le(RingBuffer *self, uint32_t x);
bool RingBuffer_write_word16be(RingBuffer *self, uint16_t x);
bool RingBuffer_write_word24be(RingBuffer *self, uint32_t x);
bool RingBuffer_write_word32be(RingBuffer *self, uint32_t x);
bool RingBuffer_write_char(RingBuffer *self, uint8_t x);

size_t RingBuffer_read(RingBuffer *self, uint8_t *data, size_t len);

void RingBuffer_end(RingBuffer *self);

#endif