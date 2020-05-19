//
// Created by yanqinming on 2020/5/19.
//


#include "includes/utils/FFmpegUtils.h"


AVFormatContext *FFmpegUtils::avFormatContext;

int FFmpegUtils::width;
int FFmpegUtils::height;
enum AVPixelFormat FFmpegUtils::pix_fmt;

AVCodecContext *FFmpegUtils::videoDecodeContext;
int FFmpegUtils::videoStreamIndex;
AVStream *FFmpegUtils::videoStream;
const char *FFmpegUtils::videoOutputFilePath;
FILE *FFmpegUtils::videoOutPutFile;

AVCodecContext *FFmpegUtils::audioDecodeContext;
int FFmpegUtils::audioStreamIndex;
AVStream *FFmpegUtils::audioStream;
const char *FFmpegUtils::audioOutputFilePath;
FILE *FFmpegUtils::audioOutPutFile;

AVFrame *FFmpegUtils::frame = NULL;
AVPacket FFmpegUtils::packet;
uint8_t *FFmpegUtils::videoDstData[4];
int FFmpegUtils::videoDstLinesize[4];
int FFmpegUtils::videoDstBufsize;
int FFmpegUtils::videoFrameCount = 0;
int FFmpegUtils::audioFrameCount = 0;
const char *FFmpegUtils::srcFilePath;

int FFmpegUtils::demuxing_decoding(const char *srcFilePath, const char *audioOutputFilePath,
                                   const char *videoOutputFilePath) {
    FFmpegUtils::srcFilePath = srcFilePath;
    FFmpegUtils::audioOutputFilePath = audioOutputFilePath;
    FFmpegUtils::videoOutputFilePath = videoOutputFilePath;
    // 用于保存处理结果
    int ret = 0;
    // 1.注册编解码器
    av_register_all();
    // 2.打开文件，并初始化 avFormatContext
    LOGD("加载源文件（%s）。。。\n", srcFilePath);
    ret = avformat_open_input(&avFormatContext, srcFilePath, NULL, NULL);
    if (ret < 0) {
        LOGE("无法打开源文件（%s）！！！\n", srcFilePath);
        goto end;
    }
    // 3.检索文件中的流信息，保存在 avFormatContext 中
    LOGD("开始从文件中查找音视频流。。。\n");
    ret = avformat_find_stream_info(avFormatContext, NULL);
    if (ret < 0) {
        LOGE("无法从源文件（%s）中找到正确的流信息。\n", srcFilePath);
        goto end;
    }
    // 4.准备开始解码音视频
    LOGD("开始查找和创建对应的解码器。。。\n");
    // 4.1 初始化视频解码器
    ret = open_codec_context(&videoStreamIndex, &videoDecodeContext, avFormatContext,
                             AVMEDIA_TYPE_VIDEO);
    if (ret >= 0) {
        videoStream = avFormatContext->streams[videoStreamIndex];
        videoOutPutFile = fopen(videoOutputFilePath, "wb");
        if (!videoOutPutFile) {
            LOGE("无法打开视频解码后保存的文件（%s）", videoOutputFilePath);
            ret = -1;
            goto end;
        }
        width = videoDecodeContext->width;
        height = videoDecodeContext->height;
        pix_fmt = videoDecodeContext->pix_fmt;
        ret = av_image_alloc(videoDstData, videoDstLinesize, width, height, pix_fmt, 1);
        if (ret < 0) {
            LOGE("无法创建解码视频缓存区！！！\n");
            goto end;
        }
        videoDstBufsize = ret;
    }
    // 4.2 初始化音频解码器
    ret = open_codec_context(&audioStreamIndex, &audioDecodeContext, avFormatContext,
                             AVMEDIA_TYPE_AUDIO);
    if (ret >= 0) {
        audioStream = avFormatContext->streams[audioStreamIndex];
        audioOutPutFile = fopen(audioOutputFilePath, "wb");
        if (!audioOutPutFile) {
            LOGE("无法打开音频解码后保存的文件（%s）", audioOutputFilePath);
            ret = -1;
            goto end;
        }
    }
    // 打印输入文件详细的格式信息
    av_dump_format(avFormatContext, 0, srcFilePath, 0);
    if (!audioStream && !videoStream) {
        LOGE("找不到对应的音频或视频流！！！");
        ret = -1;
        goto end;
    }
    // 4.3 初始化 frame 和 packet
    frame = av_frame_alloc();
    if (!frame) {
        LOGE("初始化 frame 失败， 内存不足");
        ret = AVERROR(ENOMEM);
        goto end;
    }
    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    // 5. 开始解码音视频
    LOGD("开始解码音视频。。。");
    while (av_read_frame(avFormatContext, &packet) >= 0) {
        if (packet.stream_index == videoStreamIndex) {
            ret = decode_packet(videoDecodeContext, &packet);
        } else if (packet.stream_index == audioStreamIndex) {
            ret = decode_packet(audioDecodeContext, &packet);
        }
        av_packet_unref(&packet);
        if (ret < 0) {
            break;
        }
    }
    ret = 0;

    // 6. flush the decoders
    if (videoDecodeContext) {
        decode_packet(videoDecodeContext, NULL);
    }
    if (audioDecodeContext) {
        decode_packet(audioDecodeContext, NULL);
    }

    end:
    avcodec_free_context(&videoDecodeContext);
    avcodec_free_context(&audioDecodeContext);
    avformat_close_input(&avFormatContext);
    if (audioOutPutFile) {
        fclose(audioOutPutFile);
    }
    if (videoOutPutFile) {
        fclose(videoOutPutFile);
    }
    av_frame_free(&frame);
    av_free(videoDstData[0]);

    if (ret >= 0) {
        LOGD("解码成功！！！");
    } else {
        LOGE("解码失败！！！");
    }

    return ret;
}

int FFmpegUtils::decode_packet(AVCodecContext *dec, const AVPacket *pkt) {
    int ret = 0;

    // submit the packet to the decoder
    ret = avcodec_send_packet(dec, pkt);
    if (ret < 0) {
        LOGE("Error submitting a packet for decoding (%s)\n", av_err2str(ret));
        return ret;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(dec, frame);
        if (ret < 0) {
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
                return 0;
            }

            LOGE("解码过程中，发现%s错误！！！", av_err2str(ret));
            return ret;
        }

        if (dec->codec_type == AVMEDIA_TYPE_VIDEO) {
            ret = output_video_frame(frame);
        } else if (dec->codec_type == AVMEDIA_TYPE_AUDIO){
            ret = output_audio_frame(frame);
        }

        av_frame_unref(frame);
        if (ret < 0) {
            return ret;
        }
    }

    return 0;
}

int
FFmpegUtils::open_codec_context(int *streamIdx, AVCodecContext **decCtx, AVFormatContext *fmtCtx,
                                enum AVMediaType type) {
    int ret;
    int streamIndex;
    AVStream *avStream;
    AVCodec *avCodec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(avFormatContext, type, -1, -1, NULL, 0);
    if (ret < 0) {
        LOGE("从源文件（%s）中找不到%s流", srcFilePath, av_get_media_type_string(type));
        return ret;
    } else {
        streamIndex = ret;
        avStream = avFormatContext->streams[streamIndex];

        avCodec = avcodec_find_decoder(avStream->codecpar->codec_id);
        if (!avCodec) {
            LOGE("无法找到对应的解码器（%d）！！！", avStream->codecpar->codec_id);
            return AVERROR(EINVAL);
        }
        *decCtx = avcodec_alloc_context3(avCodec);
        if (!*decCtx) {
            LOGE("无法创建解码上下文，内存不足！！！", avStream->codecpar->codec_id);
            return AVERROR(ENOMEM);
        }
        ret = avcodec_parameters_to_context(*decCtx, avStream->codecpar);
        if (ret < 0) {
            LOGE("无法拷贝解码器参数到上下文中！！！");
            return ret;
        }
        ret = avcodec_open2(*decCtx, avCodec, &opts);
        if (ret < 0) {
            LOGE("无法正常加载解码器！！！");
            return ret;
        }
        *streamIdx = streamIndex;
    }
    return 0;
}

int FFmpegUtils::output_video_frame(AVFrame *frame) {
    if (frame->width != width || frame->height != height ||
        frame->format != pix_fmt) {
        /* To handle this change, one could call av_image_alloc again and
         * decode the following frames into another rawvideo file. */
        LOGE("Error: Width, height and pixel format have to be "
                        "constant in a rawvideo file, but the width, height or "
                        "pixel format of the input video changed:\n"
                        "old: width = %d, height = %d, format = %s\n"
                        "new: width = %d, height = %d, format = %s\n",
                width, height, av_get_pix_fmt_name(pix_fmt),
                frame->width, frame->height,
                av_get_pix_fmt_name(frame->format));
        return -1;
    }

    LOGD("video_frame n:%d coded_n:%d\n",
           videoFrameCount++, frame->coded_picture_number);

    /* copy decoded frame to destination buffer:
     * this is required since rawvideo expects non aligned data */
    av_image_copy(videoDstData, videoDstLinesize,
                  (const uint8_t **)(frame->data), frame->linesize,
                  pix_fmt, width, height);

    /* write to rawvideo file */
    fwrite(videoDstData[0], 1, videoDstBufsize, videoOutPutFile);
    return 0;
}

int FFmpegUtils::output_audio_frame(AVFrame *frame) {
    size_t unpadded_linesize = static_cast<size_t>(frame->nb_samples * av_get_bytes_per_sample(
            static_cast<AVSampleFormat>(frame->format)));
    LOGD("audio_frame n:%d nb_samples:%d pts:%s\n",
           audioFrameCount++, frame->nb_samples,
           av_ts2timestr(frame->pts, &audioDecodeContext->time_base));

    /* Write the raw audio data samples of the first plane. This works
     * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
     * most audio decoders output planar audio, which uses a separate
     * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
     * In other words, this code will write only the first audio channel
     * in these cases.
     * You should use libswresample or libavfilter to convert the frame
     * to packed data. */
    fwrite(frame->extended_data[0], 1, unpadded_linesize, audioOutPutFile);

    return 0;
}

