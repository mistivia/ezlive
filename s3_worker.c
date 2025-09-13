#include "s3_worker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ezlive_config.h"
#include "task_queue.h"
#include "s3_client.h"

TaskQueue task_queue;

void exec_s3_task(void *vtask) {
    char obj_name_buf[256];
    S3Task *task = vtask;
    if (task->task_type == kUploadTask) {
        snprintf(obj_name_buf, 255, "%s%s", ezlive_config->s3_path, task->remote_name);
        S3Client_put(task->local_file, obj_name_buf);
        remove(task->local_file);
    } else if (task->task_type == kDeleteTask) {
        snprintf(obj_name_buf, 255, "%s%s", ezlive_config->s3_path, task->remote_name);
        S3Client_delete(obj_name_buf);
    } else if (task->task_type == kClearTask) {
        // TODO
    } else {
        fprintf(stderr, "unknown task type.\n");
    }
    free(task->local_file);
    free(task->remote_name);
    free(task);
}

void s3_worker_init() {
    S3Client_init();
    TaskQueue_init(&task_queue, 128);
}

void s3_worker_push(S3Task task) {
    S3Task *ptask = malloc(sizeof(S3Task));
    *ptask = task;
    ptask->local_file = ptask->local_file;
    ptask->remote_name = ptask->remote_name;
    TaskQueue_push(&task_queue, exec_s3_task, ptask);
}

void* s3_worker_main(void *ctx) {
    while (1) {
        TaskFn task_fn;
        void *arg;
        TaskQueue_pop(&task_queue, &task_fn, &arg);
        (*task_fn)(arg);
    }
}

S3Task s3_upload_task(const char *local, const char *remote) {
    return (S3Task) {
        .task_type = kUploadTask,
        .local_file = strdup(local),
        .remote_name = strdup(remote),
    };
}

S3Task s3_delete_task(const char *name) {
    return (S3Task) {
        .task_type = kDeleteTask,
        .local_file = NULL,
        .remote_name = strdup(name),
    };
}

S3Task s3_clear_task() {
    return (S3Task) {
        .task_type = kClearTask,
        .local_file = NULL,
        .remote_name = NULL,
    };
}