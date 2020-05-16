//
// Created by yanqinming on 2020/5/15.
//

#ifndef FFMPEGDEMO_AVFORMAT_H
#define FFMPEGDEMO_AVFORMAT_H

extern "C" {
//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#include <math.h>
//#include <libavutil/channel_layout.h>   // 用户音频声道布局操作
//#include <libavutil/opt.h>  // 设置操作选项操作
//#include <libavutil/mathematics.h>  // 用于数学相关操作
//#include <libavutil/timestamp.h>    // 用于时间戳操作
//#include <libavformat/avformat.h>   // 用于封装与解封装操作
//#include <libswscale/swscale.h>     // 用于缩放、转换颜色格式操作
//#include <libswresample/swresample.h>   // 用于进行音频采样率操作
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <utils/Log.h>

#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096
};

class AVFormat {
    public:
        AVFormatContext *avFormatContext;
        AVCodecContext *video_dec_ctx = NULL;
        AVCodecContext *audio_dec_ctx = NULL;
        AVStream *video_stream = NULL;
        AVStream *audio_stream = NULL;
        FILE *video_dst_file = NULL;
        FILE *audio_dst_file = NULL;
        int video_stream_idx = -1;
        int audio_stream_idx = -1;
        AVFrame *frame = NULL;
        AVPacket pkt;
        int video_frame_count = 0;
        int audio_frame_count = 0;
        // 音视频流封装
        void muxing();
        // 音视频流解封装解码
        bool demuxing_decoding(const char* srcfile, const char* audio_output_file, const char* video_output_file);
        int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, AVMediaType type);
        int decode_packet(int *got_frame, int cached);
        int output_audio_frame(AVFrame *frame);
        int get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt);
};


#endif //FFMPEGDEMO_AVFORMAT_H
