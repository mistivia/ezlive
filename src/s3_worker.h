#ifndef S3_WORKER_H_
#define S3_WORKER_H_

typedef enum {
    kUploadTask,
    kDeleteTask,
    kClearTask,
} S3TaskType;

typedef struct {
    S3TaskType task_type;
    char *local_file;
    char *remote_name;
} S3Task;

S3Task s3_upload_task(const char *local, const char *remote);

S3Task s3_delete_task(const char *name);

S3Task s3_clear_task();

void exec_s3_task(void *);

void s3_worker_init();

void* s3_worker_main(void *);

void s3_worker_push(S3Task task);

#endif
