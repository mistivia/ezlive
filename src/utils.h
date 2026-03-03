#pragma once

#include <functional>

namespace ezlive {

template<typename F>
class defer {
public:
    explicit defer(F f) : m_func(f), m_active(true) {}
    
    ~defer() { if (m_active) { m_func(); } }
    
    defer(const defer&) = delete;
    
    defer& operator=(const defer&) = delete;
    
    defer(defer&& other)
        : m_func(std::move(other.m_func))
        , m_active(other.m_active)
    {
        other.m_active = false;
    }
    
    void cancel() { m_active = false; }

private:
    F m_func;
    bool m_active;
};

void tmp_local_filename(const char *prefix, char *buf);

void tmp_ts_prefix(char *buf);

void ts_filename(const char *prefix, int num, char *buf);

void upload_file(const char *local, const char *remote);

void remove_remote(const char *remote);

} // namespace ezlive