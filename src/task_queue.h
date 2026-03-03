#pragma once

#include <functional>
#include <vector>
#include <mutex>
#include <condition_variable>

namespace ezlive {

class task_queue {
public:
    using task_fn = std::function<void(void*)>;

    explicit task_queue(int capacity);
    ~task_queue();

    void push(task_fn task, void* arg);
    void pop(task_fn& task, void** arg);

private:
    std::vector<task_fn> m_tasks;
    std::vector<void*> m_args;
    int m_capacity;
    int m_front;
    int m_end;
    int m_size;
    std::mutex m_lock;
    std::condition_variable m_not_full;
    std::condition_variable m_not_empty;
};

} // namespace ezlive
