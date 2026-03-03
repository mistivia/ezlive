#include "s3_worker.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "ezlive_config.h"
#include "task_queue.h"
#include "s3_client.h"

namespace ezlive {

task_queue tq{128};

void exec_s3_task(void *vtask)
{
    char obj_name_buf[256] = {0};
    s3_task *task = static_cast<s3_task*>(vtask);
    if (task->task_type == s3_task_type::upload) {
        snprintf(obj_name_buf, 255, "%s%s", g_config->s3_path.c_str(), task->remote_name);
        s3_client::get_instance().put(task->local_file, obj_name_buf);
        remove(task->local_file);
    } else if (task->task_type == s3_task_type::remove) {
        snprintf(obj_name_buf, 255, "%s%s", g_config->s3_path.c_str(), task->remote_name);
        s3_client::get_instance().remove(obj_name_buf);
    } else if (task->task_type == s3_task_type::clear) {
        s3_client::get_instance().clear();
    } else {
        fprintf(stderr, "unknown task type.\\n");
    }
    free(task->local_file);
    free(task->remote_name);
    free(task);
}

void s3_worker_push(s3_task task)
{
    s3_task *ptask = static_cast<s3_task*>(malloc(sizeof(s3_task)));
    *ptask = task;
    tq.push(exec_s3_task, ptask);
}

void* s3_worker_main(void *ctx)
{
    while (1) {
        task_queue::task_fn fn;
        void *arg;
        tq.pop(fn, &arg);
        fn(arg);
    }
}

s3_task s3_upload_task(const char *local, const char *remote)
{
    return s3_task {
        s3_task_type::upload,
        strdup(local),
        strdup(remote),
    };
}

s3_task s3_delete_task(const char *name)
{
    return s3_task {
        s3_task_type::remove,
        nullptr,
        strdup(name),
    };
}

s3_task s3_clear_task()
{
    return s3_task {
        s3_task_type::clear,
        nullptr,
        nullptr,
    };
}

} // namespace ezlive
