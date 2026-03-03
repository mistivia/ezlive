#pragma once

#include <cstdlib>

namespace ezlive {

class srt_callback {
public:
    virtual void on_srt_start() = 0;
    
    virtual void on_srt_stop() = 0;
    
    virtual void on_srt_data(char *buf, size_t size) = 0;
};

void start_srt_server(srt_callback *cbs);

} // namespace ezlive