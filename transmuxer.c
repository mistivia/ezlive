#include "transmuxer.h"
#include "ringbuf.h"

#include <bits/pthreadtypes.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/mem.h>
#include <libavutil/timestamp.h>
#include <pthread.h>
#include <stdlib.h>


static int wait_for_new_stream(Transmuxer *self) {
    while (pthread_cond_wait(&self->streaming_cond, &self->lock)) {
        if (self->quit) {
            return 0;
        }
        if (self->stream != NULL) break;
    }
    return 1;
}

typedef struct {
    AVStream *audio_stream;
    AVStream *video_stream;
} StreamPair;

#define OUT_VIDEO_STREAM_INDEX 0
#define OUT_AUDIO_STREAM_INDEX 1

static StreamPair start_new_output_file(
        AVFormatContext *in_fmt_ctx,
        AVFormatContext **out_fmt_ctx,
        const char *out_filename,
        int aidx, int vidx) {
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
        .audio_stream = out_audio_stream,
        .video_stream = out_video_stream,
    };
}

static int RingBuffer_avio_read(void *ctx, uint8_t *buf, int buf_size) {
    RingBuffer *rb = ctx;
    size_t n = RingBuffer_read(rb, buf, buf_size);
    if (n == 0 && buf_size > 0) {
        return AVERROR_EOF;
    }
    return (int)n;
}

AVIOContext* create_avio_from_ringbuffer(RingBuffer *rb, int buffer_size) {
    uint8_t *avio_buf = av_malloc(buffer_size);
    AVIOContext *avio = avio_alloc_context(
        avio_buf, buffer_size,
        0,
        rb,
        RingBuffer_avio_read,
        NULL,
        NULL);
    return avio;
}

static void finalize_output_file(AVFormatContext *out_fmt_ctx) {
    av_write_trailer(out_fmt_ctx);
    if (!(out_fmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&out_fmt_ctx->pb);
    }
    avformat_free_context(out_fmt_ctx);
    // TODO: update m3u8
}

#define SEGMENT_DURATION 5

void* Transmuxer_main (void *vself) {
    Transmuxer *self = vself;
    pthread_mutex_lock(&self->lock);
    while (wait_for_new_stream(self)) {
        AVFormatContext *in_fmt_ctx = avformat_alloc_context();
        in_fmt_ctx->pb = create_avio_from_ringbuffer(self->stream, 4096);
        const AVInputFormat *flv_fmt = av_find_input_format("flv");
        if (avformat_open_input(&in_fmt_ctx, NULL, flv_fmt, NULL) < 0) {
            fprintf(stderr, "Could not open input file\n");
            abort();
        }

        if (avformat_find_stream_info(in_fmt_ctx, NULL) < 0) {
            fprintf(stderr, "Could not find stream info\n");
            abort();
        }

        int video_stream_index = -1, audio_stream_index = -1;
        for (int i = 0; i < in_fmt_ctx->nb_streams; i++) {
            if (in_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                video_stream_index = i;
            else if (in_fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
                audio_stream_index = i;
        }

        if (video_stream_index < 0) {
            fprintf(stderr, "No video stream found\n");
            abort();
        }
        if (audio_stream_index < 0) {
            fprintf(stderr, "No audio stream found\n");
            abort();
        }

        AVFormatContext *out_fmt_ctx = NULL;
        int segment_index = 0;
        int64_t segment_start_pts = 0;

        char out_filename[256];
        snprintf(out_filename, sizeof(out_filename), "segment_%03d.ts", segment_index);


        int64_t pts_time;
        AVPacket pkt;
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
                    finalize_output_file(out_fmt_ctx);
                    
                    // open new ts
                    segment_index++;
                    segment_start_pts = pts_time;
                    snprintf(out_filename, sizeof(out_filename), "segment_%03d.ts", segment_index);
                    output_stream = start_new_output_file(in_fmt_ctx, &out_fmt_ctx, out_filename, audio_stream_index, video_stream_index);
                }
            }
            pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
            pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX);
            pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
            pkt.pos = -1;
            pkt.stream_index = (pkt.stream_index == audio_stream_index) ? OUT_AUDIO_STREAM_INDEX : OUT_VIDEO_STREAM_INDEX;

            av_interleaved_write_frame(out_fmt_ctx, &pkt);
            av_packet_unref(&pkt);
        }
        printf("new ts: %ld\n", pts_time - segment_start_pts);
        finalize_output_file(out_fmt_ctx);

        av_free(in_fmt_ctx->pb->buffer);
        avio_context_free(&in_fmt_ctx->pb);
        avformat_close_input(&in_fmt_ctx);
        RingBuffer_destroy(self->stream);
        self->stream = NULL;
        free(self->stream);
    }
    pthread_mutex_unlock(&self->lock);
    return NULL;
}

void Transmuxer_init(Transmuxer *self) {
    pthread_mutex_init(&self->lock, NULL);
    pthread_cond_init(&self->streaming_cond, NULL);
    self->stream = NULL;
    self->quit = false;
}

void Transmuxer_new_stream(Transmuxer *self, RingBuffer *ringbuf) {
    pthread_mutex_lock(&self->lock);
    self->stream = ringbuf;
    pthread_mutex_unlock(&self->lock);
    pthread_cond_signal(&self->streaming_cond);
}
