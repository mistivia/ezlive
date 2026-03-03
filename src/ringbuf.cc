#include "ringbuf.h"

#include <mutex>
#include <cstring>

namespace ezlive {

void ring_buffer::stop() {
    std::unique_lock<std::mutex> lk{m_lock};
    m_finished_flag = true;
    m_not_empty.notify_one();
}

size_t ring_buffer::write(const uint8_t *data, size_t len) {
    size_t written = 0;
    std::unique_lock<std::mutex> lk{m_lock};
    while (written < len) {
        while (m_full_flag) {
            m_not_full.wait(lk);
        }

        size_t free_space = space();
        size_t to_write = (len - written > free_space) ? free_space : len - written;

        size_t first = (to_write > m_buffer.size() - m_head) ? m_buffer.size() - m_head : to_write;
        memcpy(&m_buffer[0] + m_head, data + written, first);

        size_t second = to_write - first;
        if (second > 0) memcpy(&m_buffer[0], data + written + first, second);

        m_head = (m_head + to_write) % m_buffer.size();
        if (to_write == free_space) m_full_flag = true;

        written += to_write;
        m_not_empty.notify_one();
    }
    m_size += written;
    return written;
}

size_t ring_buffer::read(uint8_t *data, size_t len) {
    size_t read = 0;
    std::unique_lock<std::mutex> lk{m_lock};

    while (read < len) {
        while (m_size == 0) {
            if (m_finished_flag) {
                goto end;
            }
            m_not_empty.wait(lk);
        }
        size_t available = size();
        size_t to_read = (len - read > available) ? available : len - read;

        size_t first = (to_read > m_buffer.size() - m_tail) ? m_buffer.size() - m_tail : to_read;
        memcpy(data + read, &m_buffer[0] + m_tail, first);

        size_t second = to_read - first;
        if (second > 0) memcpy(data + read + first, &m_buffer[0], second);

        m_tail = (m_tail + to_read) % m_buffer.size();
        m_full_flag = false;

        read += to_read;
        m_not_full.notify_one();
    }
end:
    m_size -= read;
    return read;
}


} // namespace ezlive