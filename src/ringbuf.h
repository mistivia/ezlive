#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ezlive {

class ring_buffer {
public:
    ring_buffer(size_t sz);
    ring_buffer(const ring_buffer&) = delete;
    ring_buffer& operator=(const ring_buffer&) = delete;
    size_t size() { return m_size; }
    size_t space() { return m_buffer.size() - m_size; }
    size_t write(const uint8_t *data, size_t len);
    size_t read(uint8_t *data, size_t len);
    void stop();
private:
    std::vector<uint8_t> m_buffer;
    int64_t m_size = 0;
    int64_t m_head = 0;
    int64_t m_tail = 0;
    bool m_full_flag = false;
    bool m_finished_flag = false;
    std::mutex m_lock;
    std::condition_variable m_not_empty;
    std::condition_variable m_not_full;
};

} // ezlive