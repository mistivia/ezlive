#ifndef TASK_QUEUE_H_
#define TASK_QUEUE_H_

#include <pthread.h>

typedef void (*TaskFn)(void *arg);

typedef struct {
    TaskFn *tasks;
    void **args;
    int capacity;
    int front;
    int end;
    int size;
    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} TaskQueue;

void TaskQueue_init(TaskQueue *self, int capacity);

void TaskQueue_destroy(TaskQueue *self);

void TaskQueue_push(TaskQueue *self, TaskFn task, void *arg);

void TaskQueue_pop(TaskQueue *self, TaskFn *task, void **arg);

#endif
