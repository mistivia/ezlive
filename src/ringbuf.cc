#include "ringbuf.h"

#include <mutex>
#include <cstring>

namespace ezlive {

ring_buffer::ring_buffer(size_t sz)
    : m_buffer(sz, 0)
{
}

void ring_buffer::stop()
{
    std::unique_lock<std::mutex> lk{m_lock};
    m_finished_flag = true;
    m_not_empty.notify_all();
}

size_t ring_buffer::write(const uint8_t *data, size_t len)
{
    size_t written = 0;
    std::unique_lock<std::mutex> lk{m_lock};
    while (written < len) {
        m_not_full.wait(lk, [this]() { return !m_full_flag; });

        size_t free_space = space();
        size_t to_write = (len - written > free_space) ? free_space : len - written;

        size_t first = (to_write > m_buffer.size() - m_head) ? m_buffer.size() - m_head : to_write;
        memcpy(&m_buffer[0] + m_head, data + written, first);

        size_t second = to_write - first;
        if (second > 0) memcpy(&m_buffer[0], data + written + first, second);

        m_head = (m_head + to_write) % m_buffer.size();
        if (to_write == free_space) m_full_flag = true;

        written += to_write;
        m_size += to_write;
        m_not_empty.notify_one();
    }
    return written;
}

size_t ring_buffer::read(uint8_t *data, size_t len)
{
    std::unique_lock<std::mutex> lk{m_lock};

    // Wait for data to be available (unless buffer is already finished)
    if (m_size == 0) {
        m_not_empty.wait(lk, [this]() { return m_size > 0 || m_finished_flag; });
    }
    
    if (m_size == 0) {
        // Buffer empty and finished flag set
        return 0;
    }
    
    // Read up to min(len, available) bytes, then return
    size_t available = m_size;
    size_t to_read = (len > available) ? available : len;

    size_t first = (to_read > m_buffer.size() - m_tail) ? m_buffer.size() - m_tail : to_read;
    memcpy(data, &m_buffer[0] + m_tail, first);

    size_t second = to_read - first;
    if (second > 0) memcpy(data + first, &m_buffer[0], second);

    m_tail = (m_tail + to_read) % m_buffer.size();
    m_full_flag = false;

    m_size -= to_read;
    m_not_full.notify_one();
    
    return to_read;
}


} // namespace ezlive
