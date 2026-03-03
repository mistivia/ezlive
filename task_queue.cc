#include "task_queue.h"

namespace ezlive {

task_queue::task_queue(int capacity)
    : m_tasks(capacity)
    , m_args(capacity)
    , m_capacity(capacity)
    , m_front(0)
    , m_end(0)
    , m_size(0)
{
}

task_queue::~task_queue()
{
}

void task_queue::push(task_fn task, void* arg)
{
    std::unique_lock<std::mutex> lock(m_lock);
    m_not_full.wait(lock, [this] { return m_size < m_capacity; });
    m_tasks[m_end] = task;
    m_args[m_end] = arg;
    m_end = (m_end + 1) % m_capacity;
    m_size++;
    lock.unlock();
    m_not_empty.notify_one();
}

void task_queue::pop(task_fn& task, void** arg)
{
    std::unique_lock<std::mutex> lock(m_lock);
    m_not_empty.wait(lock, [this] { return m_size > 0; });
    task = m_tasks[m_front];
    *arg = m_args[m_front];
    m_front = (m_front + 1) % m_capacity;
    m_size--;
    lock.unlock();
    m_not_full.notify_one();
}


} // namespace ezlive