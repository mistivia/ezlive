#include "ringbuf.h"

#include <string.h>

void RingBuffer_init(RingBuffer *self, size_t size) {
    self->buffer = (uint8_t *)malloc(size);
    if (!self->buffer) {
        return;
    }

    self->max = size;
    self->head = 0;
    self->tail = 0;
    self->full_flag = false;
    self->finished_flag = false;

    pthread_mutex_init(&self->mutex, NULL);
    pthread_cond_init(&self->not_empty_cond, NULL);
    pthread_cond_init(&self->not_full_cond, NULL);
}

void RingBuffer_destroy(RingBuffer *self) {
    free(self->buffer);
    pthread_mutex_destroy(&self->mutex);
    pthread_cond_destroy(&self->not_empty_cond);
    pthread_cond_destroy(&self->not_full_cond);
}

size_t RingBuffer_size(RingBuffer *self) {
    size_t size;
    if (self->full_flag) {
        size = self->max;
    } else if (self->head >= self->tail) {
        size = self->head - self->tail;
    } else {
        size = self->max + self->head - self->tail;
    }
    return size;
}

size_t RingBuffer_space(RingBuffer *self) {
    return self->max - RingBuffer_size(self);
}

void RingBuffer_end(RingBuffer *self) {
    pthread_mutex_lock(&self->mutex);
    self->finished_flag = true;
    pthread_mutex_unlock(&self->mutex);
}

size_t RingBuffer_write(RingBuffer *self, const uint8_t *data, size_t len) {
    size_t written = 0;
    pthread_mutex_lock(&self->mutex);
    while (written < len) {
        while (self->full_flag) {
            pthread_cond_wait(&self->not_full_cond, &self->mutex);
        }

        size_t free_space = RingBuffer_space(self);
        size_t to_write = (len - written > free_space) ? free_space : len - written;

        size_t first = (to_write > self->max - self->head) ? self->max - self->head : to_write;
        memcpy(self->buffer + self->head, data + written, first);

        size_t second = to_write - first;
        if (second > 0) memcpy(self->buffer, data + written + first, second);

        self->head = (self->head + to_write) % self->max;
        if (to_write == free_space) self->full_flag = true;

        written += to_write;

        pthread_cond_signal(&self->not_empty_cond);
    }
    pthread_mutex_unlock(&self->mutex);
    return written;
}

size_t RingBuffer_read(RingBuffer *self, uint8_t *data, size_t len) {
    size_t read = 0;
    pthread_mutex_lock(&self->mutex);

    while (read < len) {
        while (RingBuffer_size(self) == 0) {
            if (self->finished_flag) {
                goto end;
            }
            pthread_cond_wait(&self->not_empty_cond, &self->mutex);
        }

        size_t available = RingBuffer_size(self);
        size_t to_read = (len - read > available) ? available : len - read;

        size_t first = (to_read > self->max - self->tail) ? self->max - self->tail : to_read;
        memcpy(data + read, self->buffer + self->tail, first);

        size_t second = to_read - first;
        if (second > 0) memcpy(data + read + first, self->buffer, second);

        self->tail = (self->tail + to_read) % self->max;
        self->full_flag = false;

        read += to_read;

        pthread_cond_signal(&self->not_full_cond);
    }
end:
    pthread_mutex_unlock(&self->mutex);
    return read;
}

bool RingBuffer_write_word16le(RingBuffer* self, uint16_t x) {
    uint8_t buf[2];
    buf[0] = x & 0xff;
    buf[1] = (x >> 8) & 0xff;
    int r = RingBuffer_write(self, buf, 2);
    if (r != 2) return false;
    return true;
}

bool RingBuffer_write_word32le(RingBuffer* self, uint32_t x) {
    uint8_t buf[4];
    buf[0] = x & 0xff;
    buf[1] = (x >> 8) & 0xff;
    buf[2] = (x >> 16) & 0xff;
    buf[3] = (x >> 24) & 0xff;
    int r = RingBuffer_write(self, buf, 4);
    if (r != 4) return false;
    return true;
}

bool RingBuffer_write_word16be(RingBuffer* self, uint16_t x) {
    uint8_t buf[2];
    buf[1] = x & 0xff;
    buf[0] = (x >> 8) & 0xff;
    int r = RingBuffer_write(self, buf, 2);
    if (r != 2) return false;
    return true;
}

bool RingBuffer_write_word32be(RingBuffer* self, uint32_t x) {
    uint8_t buf[4];
    buf[3] = x & 0xff;
    buf[2] = (x >> 8) & 0xff;
    buf[1] = (x >> 16) & 0xff;
    buf[0] = (x >> 24) & 0xff;
    int r = RingBuffer_write(self, buf, 4);
    if (r != 4) return false;
    return true;
}

bool RingBuffer_write_word24le(RingBuffer* self, uint32_t x) {
    uint8_t buf[3];
    buf[0] = x & 0xff;
    buf[1] = (x >> 8) & 0xff;
    buf[2] = (x >> 16) & 0xff;
    int r = RingBuffer_write(self, buf, 3);
    if (r != 3) return false;
    return true;
}

bool RingBuffer_write_word24be(RingBuffer* self, uint32_t x) {
    uint8_t buf[3];
    buf[2] = x & 0xff;
    buf[1] = (x >> 8) & 0xff;
    buf[0] = (x >> 16) & 0xff;
    int r = RingBuffer_write(self, buf, 3);
    if (r != 3) return false;
    return true;
}

bool RingBuffer_write_char(RingBuffer* self, uint8_t x) {
    uint8_t buf[1];
    buf[0] = x;
    int r = RingBuffer_write(self, buf, 1);
    return r == 1;
}
