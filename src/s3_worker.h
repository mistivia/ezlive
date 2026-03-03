#pragma once

#include <string>

namespace ezlive {

enum class s3_task_type {
    upload,
    remove,
    clear
};

struct s3_task {
    s3_task_type task_type;
    char *local_file;
    char *remote_name;
};

s3_task s3_upload_task(const char *local, const char *remote);
s3_task s3_delete_task(const char *name);
s3_task s3_clear_task();

void exec_s3_task(void *);

void* s3_worker_main(void *);

void s3_worker_push(s3_task task);

} // namespace ezlive
