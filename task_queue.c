#include "task_queue.h"

#include <stdlib.h>

void TaskQueue_init(TaskQueue *self, int capacity) {
    self->tasks = malloc(sizeof(TaskFn) * capacity);
    self->args = malloc(sizeof(void*) * capacity);
    self->capacity = capacity;
    self->front = 0;
    self->end = 0;
    self->size = 0;
    pthread_mutex_init(&self->lock, NULL);
    pthread_cond_init(&self->not_full, NULL);
    pthread_cond_init(&self->not_empty, NULL);
}

void TaskQueue_destroy(TaskQueue *self) {
    free(self->tasks);
    free(self->args);
    pthread_mutex_destroy(&self->lock);
    pthread_cond_destroy(&self->not_full);
    pthread_cond_destroy(&self->not_empty);
}

void TaskQueue_push(TaskQueue *self, TaskFn task, void *arg) {
    pthread_mutex_lock(&self->lock);
    while (self->size == self->capacity) {
        pthread_cond_wait(&self->not_full, &self->lock);
    }
    self->tasks[self->end] = task;
    self->args[self->end] = arg;
    self->end = (self->end + 1) % self->capacity;
    self->size++;
    pthread_cond_signal(&self->not_empty);
    pthread_mutex_unlock(&self->lock);
}

void TaskQueue_pop(TaskQueue *self, TaskFn *task, void **arg) {
    pthread_mutex_lock(&self->lock);
    while (self->size == 0) {
        pthread_cond_wait(&self->not_empty, &self->lock);
    }
    *task = self->tasks[self->front];
    *arg = self->args[self->front];
    self->front = (self->front + 1) % self->capacity;
    self->size--;
    pthread_cond_signal(&self->not_full);
    pthread_mutex_unlock(&self->lock);
}
