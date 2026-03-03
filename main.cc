#include <cstdio>
#include <memory>
#include <thread>

#include "ezlive_config.h"
#include "srtserver.h"
#include "ringbuf.h"
#include "transcode_talker.h"
#include "s3_worker.h"

namespace ezlive {

class main_ctx : public srt_callback {
public:
    void on_srt_start() override
    {
        m_ringbuf = std::make_unique<ring_buffer>(4096);
        TranscodeTalker_new_stream(&m_transmux, rb);
    }

    void on_srt_stop() override
    {
        m_ringbuf->stop();
    }

    void on_srt_data(char *buf, size_t size) override
    {
        m_ringbuf->write((const uint8_t *)buf, size);
    }

private:
    std::unique_ptr<ring_buffer> m_ringbuf;
    TranscodeTalker m_transmux;
};


void cleantmpfile() {
#ifdef _WIN32
    system("del tmp*");
#else
    system("rm /tmp/ezlive*");
#endif
}

int main(int argc, char **argv) {
    cleantmpfile();
    ezlive_config = new EZLiveConfig{};
    EZLiveConfig_init(ezlive_config);
    bool succ = {0};
    if (argc == 1) {
        succ = EZLiveConfig_load(ezlive_config, "./config");
#if defined(_WIN32)
        if (!succ) {
            succ = EZLiveConfig_load(ezlive_config, "./config.txt");
        }
#endif
        if (!succ) {
            fprintf(stderr, "Failed to load config.\n");
            return -1;
        }
    } else if (argc == 2) {
        if (!EZLiveConfig_load(ezlive_config, argv[1])) {
            fprintf(stderr, "Failed to load config.\n");
            return -1;
        }
    } else {
        fprintf(stderr, "wrong args.\n");
        exit(-1);
    }
    int ret = {0};
    if ((ret = EZLiveConfig_validate(ezlive_config)) < 0) {
        fprintf(stderr, "ezlive config error: %d.\n", ret);
        exit(-1);
    }
    srand((unsigned) time(NULL));
    main_ctx ctx{};
    s3_worker_init();
    s3_worker_push(s3_clear_task());

    std::thread transmux_thread{[&]() {
        TranscodeTalker_main(&ctx);
    }};
    std::thread s3worker_thread{[]() {
        s3_worker_main(NULL);
    }};
    
    start_srt_server(&ctx);
    return 0;
}

} // namespace ezlive