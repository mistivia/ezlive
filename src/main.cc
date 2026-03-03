#include <cstdio>
#include <memory>
#include <thread>

#include "ezlive_config.h"
#include "s3_client.h"
#include "srtserver.h"
#include "ringbuf.h"
#include "transmuxer.h"
#include "s3_worker.h"

namespace ezlive {

class main_ctx : public srt_callback {
public:
    void start_transmuxer() {
        m_transmuxer.start();
    }
    void stop_transmuxer() {
        m_transmuxer.stop();
    }

    void on_srt_start() override
    {
        m_ringbuf = std::make_unique<ring_buffer>(4096);
        m_transmuxer.new_stream(m_ringbuf.get());
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
    transmuxer m_transmuxer;
};

void cleantmpfile()
{
#ifdef _WIN32
    system("del tmp*");
#else
    system("rm /tmp/ezlive*");
#endif
}

} // namespace ezlive

using namespace ezlive;

int main(int argc, char **argv)
{
    cleantmpfile();
    g_config = std::make_unique<config>();
    bool succ = {0};
    if (argc == 1) {
        succ = (g_config->load("./config") == 0);
#if defined(_WIN32)
        if (!succ) {
            succ = (g_config->load("./config.txt") == 0);
        }
#endif
        if (!succ) {
            fprintf(stderr, "Failed to load config.\n");
            return -1;
        }
    } else if (argc == 2) {
        if (g_config->load(argv[1]) != 0) {
            fprintf(stderr, "Failed to load config.\n");
            return -1;
        }
    } else {
        fprintf(stderr, "wrong args.\n");
        exit(-1);
    }

    srand((unsigned) time(NULL));
    main_ctx ctx{};
    s3_client::get_instance().init();
    s3_worker_init();
    s3_worker_push(s3_clear_task());

    std::thread s3worker_thread{[]() {
        s3_worker_main(NULL);
    }};
    ctx.start_transmuxer();
    start_srt_server(static_cast<srt_callback*>(&ctx));
    ctx.stop_transmuxer();
    return 0;
}
