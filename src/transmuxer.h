#pragma once

#include "ringbuf.h"
#include "hls_list.h"

#include <thread>
#include <mutex>
#include <condition_variable>

namespace ezlive {

class transmuxer {
public:
    explicit transmuxer();
    ~transmuxer();
    void start();
    void stop();
    void new_stream(ring_buffer *ringbuf);
private:
    void main_loop();
    void check_timer_loop();
    int wait_for_new_stream();
    bool should_quit();
    std::mutex m_lock;
    std::condition_variable m_streaming_cond;
    ring_buffer *m_stream = nullptr;
    bool m_quit = false;
    hls_list m_lst;
    time_t m_last_updated = 0;
    std::thread m_main_thread;
    std::thread m_check_thread;
};

} //namespace ezlive
