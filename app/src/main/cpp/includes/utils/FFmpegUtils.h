//
// Created by yanqinming on 2020/5/19.
//

#ifndef FFMPEGDEMO_FFMPEGUTILS_H
#define FFMPEGDEMO_FFMPEGUTILS_H
#include <utils/Log.h>
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/timestamp.h>
};

class FFmpegUtils {
public:
    static AVFormatContext *avFormatContext;

    static int width;
    static int height;
    static enum AVPixelFormat pix_fmt;

    static AVCodecContext *videoDecodeContext;
    static int videoStreamIndex;
    static AVStream *videoStream;
    static const char *videoOutputFilePath;
    static FILE *videoOutPutFile;

    static AVCodecContext *audioDecodeContext;
    static int audioStreamIndex;
    static AVStream *audioStream;
    static const char *audioOutputFilePath;
    static FILE *audioOutPutFile;

    static AVFrame *frame;
    static AVPacket packet;
    static uint8_t *videoDstData[4];
    static int videoDstLinesize[4];
    static int videoDstBufsize;
    static int videoFrameCount;
    static int audioFrameCount;
    static const char *srcFilePath;

    static int demuxing_decoding(const char *srcFilePath, const char *audioOutputFilePath, const char *videoOutputFilePath);
    static int decode_packet(AVCodecContext *dec, const AVPacket *pkt);
    static int open_codec_context(int *streamIdx, AVCodecContext **decCtx, AVFormatContext *fmtCtx, enum AVMediaType type);
    static int output_video_frame(AVFrame *frame);
    static int output_audio_frame(AVFrame *frame);
};


#endif //FFMPEGDEMO_FFMPEGUTILS_H
