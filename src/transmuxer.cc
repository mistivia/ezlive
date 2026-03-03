#include "transmuxer.h"

#include <cstdlib>
#include <ctime>
#include <cstdio>

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <unistd.h>

extern "C" {
    #include <libavutil/mathematics.h>
    #include <libavformat/avformat.h>
    #include <libavformat/avio.h>
    #include <libavutil/mem.h>
    #include <libavutil/timestamp.h>
}

#include "utils.h"
#include "ringbuf.h"

namespace ezlive {

namespace {

typedef struct {
    AVStream *audio_stream;
    AVStream *video_stream;
} StreamPair;

#define OUT_VIDEO_STREAM_INDEX 0
#define OUT_AUDIO_STREAM_INDEX 1
#define SEGMENT_DURATION 5

StreamPair start_new_output_file(
        AVFormatContext *in_fmt_ctx,
        AVFormatContext **out_fmt_ctx,
        const char *out_filename,
        int aidx, int vidx)
{
    avformat_alloc_output_context2(out_fmt_ctx, NULL, "mpegts", out_filename);

    AVStream *out_video_stream = avformat_new_stream(*out_fmt_ctx, NULL);
    AVStream *out_audio_stream = avformat_new_stream(*out_fmt_ctx, NULL);

    avcodec_parameters_copy(out_video_stream->codecpar, in_fmt_ctx->streams[vidx]->codecpar);
    avcodec_parameters_copy(out_audio_stream->codecpar, in_fmt_ctx->streams[aidx]->codecpar);

    if (!((*out_fmt_ctx)->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&(*out_fmt_ctx)->pb, out_filename, AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "Could not open output file '%s'\n", out_filename);
            exit(-1);
        }
    }

    if (avformat_write_header(*out_fmt_ctx, NULL) < 0) {
        fprintf(stderr, "avformat_write_header failed.\n");
        abort();
    }
    return (StreamPair) {
        .audio_stream = out_video_stream,
        .video_stream = out_audio_stream,
    };
}

int ring_buffer_avio_read(void *ctx, uint8_t *buf, int buf_size)
{
    ring_buffer *rb = (ring_buffer*)ctx;
    size_t n = rb->read(buf, buf_size);
    if (n == 0 && buf_size > 0) {
        return AVERROR_EOF;
    }
    return (int)n;
}

AVIOContext* create_avio_from_ring_buffer(ring_buffer *rb, int buffer_size)
{
    uint8_t *avio_buf = (uint8_t*)av_malloc(buffer_size);
    AVIOContext *avio = avio_alloc_context(
        avio_buf, buffer_size,
        0,
        rb,
        ring_buffer_avio_read,
        NULL,
        NULL);
    return avio;
}

void finalize_output_file(AVFormatContext *out_fmt_ctx)
{
    av_write_trailer(out_fmt_ctx);
    if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&out_fmt_ctx->pb);
    }
    avformat_free_context(out_fmt_ctx);
}

} //anonymous namespace

transmuxer::transmuxer()
    : m_stream(nullptr)
    , m_quit(false)
    , m_last_updated(0)
{
}

transmuxer::~transmuxer()
{
    stop();
}

void transmuxer::start()
{
    m_main_thread = std::thread(&transmuxer::main_loop, this);
    m_check_thread = std::thread(&transmuxer::check_timer_loop, this);
}

void transmuxer::stop()
{
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_quit = true;
    }
    m_streaming_cond.notify_all();
    if (m_main_thread.joinable()) {
        m_main_thread.join();
    }
    if (m_check_thread.joinable()) {
        m_check_thread.join();
    }
}

void transmuxer::new_stream(ring_buffer *ringbuf)
{
    std::lock_guard<std::mutex> lock(m_lock);
    m_stream = ringbuf;
    m_streaming_cond.notify_one();
}

int transmuxer::wait_for_new_stream()
{
    std::unique_lock<std::mutex> lock(m_lock, std::defer_lock);
    while (true) {
        if (m_quit) {
            return 0;
        }
        if (m_stream != nullptr) {
            return 1;
        }
        m_streaming_cond.wait(lock);
    }
}

bool transmuxer::should_quit()
{
    std::lock_guard<std::mutex> lock(m_lock);
    return m_quit;
}

void transmuxer::check_timer_loop()
{
    while (!should_quit()) {
        std::this_thread::sleep_for(std::chrono::seconds(15));
        std::time_t now = std::time(nullptr);
        std::lock_guard<std::mutex> lock(m_lock);
        if (m_lst.len() > 0 && now - m_last_updated > 60) {
            for (int i = 0; i < m_lst.len(); i++) {
                remove_remote(m_lst.file(i));
            }
            m_lst.clear();
            m_lst.update_m3u8(-1);
        }
    }
}

void transmuxer::main_loop()
{
    char remote_prefix[9] = {0};
    char remote_filename[256] = {0};
    int segment_index = 0;
    tmp_ts_prefix(remote_prefix);
    std::lock_guard<std::mutex> lk{m_lock};
    while (wait_for_new_stream()) {
        AVFormatContext *in_fmt_ctx = avformat_alloc_context();
        in_fmt_ctx->pb = create_avio_from_ring_buffer(m_stream, 4096);
        const AVInputFormat *input_fmt = av_find_input_format("mpegts");
        if (avformat_open_input(&in_fmt_ctx, NULL, input_fmt, NULL) < 0) {
            fprintf(stderr, "Could not open input file\n");
            continue;
        }

        if (avformat_find_stream_info(in_fmt_ctx, NULL) < 0) {
            fprintf(stderr, "Could not find stream info\n");
            continue;
        }

        int video_stream_index = -1, audio_stream_index = -1;
        for (unsigned int i = 0; i < in_fmt_ctx->nb_streams; i++) {
            if (in_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                video_stream_index = i;
            else if (in_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
                audio_stream_index = i;
        }

        if (video_stream_index < 0) {
            fprintf(stderr, "No video stream found\n");
            continue;
        }
        if (audio_stream_index < 0) {
            fprintf(stderr, "No audio stream found\n");
            continue;
        }

        AVFormatContext *out_fmt_ctx = NULL;
        int64_t segment_start_pts = 0;

        char out_filename[256] = {0};
        tmp_local_filename(TMP_PREFIX.c_str(), out_filename);

        int64_t pts_time = {0};
        AVPacket pkt = {0};
        StreamPair output_stream = start_new_output_file(
            in_fmt_ctx, &out_fmt_ctx, out_filename, audio_stream_index, video_stream_index);
        while (av_read_frame(in_fmt_ctx, &pkt) >= 0) {
            AVStream *in_stream = in_fmt_ctx->streams[pkt.stream_index];
            AVStream *out_stream = NULL;
            if (pkt.stream_index == video_stream_index)
                out_stream = output_stream.video_stream;
            else if (pkt.stream_index == audio_stream_index)
                out_stream = output_stream.audio_stream;
            else {
                av_packet_unref(&pkt);
                continue;
            }

            pts_time = av_rescale_q(pkt.pts, in_stream->time_base, AV_TIME_BASE_Q);

            // if need split
            if (pts_time - segment_start_pts >= SEGMENT_DURATION * AV_TIME_BASE) {
                if (pkt.stream_index == video_stream_index && (pkt.flags & AV_PKT_FLAG_KEY)) {
                    // close current ts
                    printf("new ts: %ld\n", pts_time - segment_start_pts);
                    std::time_t now = std::time(nullptr);
                    m_last_updated = now;
                    finalize_output_file(out_fmt_ctx);
                    ts_filename(remote_prefix, segment_index, remote_filename);
                    upload_file(out_filename, remote_filename);
                    std::string deleted = m_lst.push(remote_filename, (pts_time - segment_start_pts) / (double)AV_TIME_BASE);
                    m_lst.update_m3u8(segment_index);
                    if (!deleted.empty()) {
                        remove_remote(deleted.c_str());
                    }
                    segment_index++;

                    // open new ts
                    segment_start_pts = pts_time;
                    tmp_local_filename(TMP_PREFIX.c_str(), out_filename);
                    output_stream = start_new_output_file(in_fmt_ctx, &out_fmt_ctx, out_filename, audio_stream_index, video_stream_index);
                    if (pkt.stream_index == video_stream_index)
                        out_stream = output_stream.video_stream;
                    else if (pkt.stream_index == audio_stream_index)
                        out_stream = output_stream.audio_stream;
                    else {
                        av_packet_unref(&pkt);
                        continue;
                    }
                }
            }
            pkt.pts = av_rescale_q_rnd(
                pkt.pts, in_stream->time_base, out_stream->time_base,
                (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt.dts = av_rescale_q_rnd(
                pkt.dts, in_stream->time_base, out_stream->time_base,
                (enum AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
            pkt.pos = -1;
            pkt.stream_index =
                (pkt.stream_index == audio_stream_index)
                    ? OUT_AUDIO_STREAM_INDEX
                    : OUT_VIDEO_STREAM_INDEX;

            av_interleaved_write_frame(out_fmt_ctx, &pkt);
            av_packet_unref(&pkt);
        }
        printf("new ts: %ld\n", pts_time - segment_start_pts);

        finalize_output_file(out_fmt_ctx);
        m_last_updated = std::time(nullptr);
        ts_filename(remote_prefix, segment_index, remote_filename);
        upload_file(out_filename, remote_filename);
        std::string deleted = m_lst.push(remote_filename, (pts_time - segment_start_pts) / (double)AV_TIME_BASE);
        m_lst.update_m3u8(segment_index);
        if (!deleted.empty()) {
            remove_remote(deleted.c_str());
        }
        segment_index++;

        av_free(in_fmt_ctx->pb->buffer);
        avio_context_free(&in_fmt_ctx->pb);
        avformat_close_input(&in_fmt_ctx);
        m_stream = nullptr;
    }
}

} //namespace ezlive
