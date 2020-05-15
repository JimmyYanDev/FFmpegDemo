//
// Created by yanqinming on 2020/5/15.
//

#include "includes/main/AVFormat.h"

void AVFormat::demuxing_decoding(const char *srcfile, const char *audio_output_file,
                                 const char *video_output_file) {
    av_register_all();

    if (avformat_open_input(&avFormatContext, srcfile, NULL, NULL) < 0) {
        LOGE("Cannot open source file: %s.\n", srcfile);
        return;
    }

    if (avformat_find_stream_info(avFormatContext, NULL) < 0) {
        LOGE("Could not find stream infomation.\n");
        return;
    }

    if (open_codec_context(&audio_stream_idx, &audio_dec_ctx, avFormatContext, AVMEDIA_TYPE_AUDIO) >= 0) {
        audio_stream = avFormatContext->streams[audio_stream_idx];
        audio_dst_file = fopen(audio_output_file, "wb");
        if (!audio_dst_file) {
            LOGE("Could not open destination file %s\n", audio_output_file);
            goto end;
        }
    }

    // dump input information to stderr
    av_dump_format(avFormatContext, 0, srcfile, 0);

    if (!audio_stream) {
        LOGE("Could not find audio stream in the input, aborting.\n");
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame) {
        LOGE("Could not allocate frame.\n");
        goto end;
    }

    // initialize packet, set data to NULL, let the demuxer fill it
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    if (!&pkt) {
        LOGE("Failed to inir packet!\n");
    }

    if (audio_stream) {
        LOGD("Demuxing audio from file '%s' into '%s'.\n", srcfile, audio_output_file);
    }

    // read frames from the file
    while (av_read_frame(avFormatContext, &pkt) >= 0) {
        if (pkt.stream_index == audio_stream_idx) {
            int ret = decode_packet(audio_dec_ctx, &pkt);
            av_packet_unref(&pkt);
            if (ret < 0) {
                break;
            }
        }
    }

    // flush the decoders
    if (audio_dec_ctx) {
        decode_packet(audio_dec_ctx, NULL);
    }
    LOGD("Demuxing succeeded.\n");

    end:
    avcodec_free_context(&video_dec_ctx);
    avcodec_free_context(&audio_dec_ctx);
    avformat_close_input(&avFormatContext);
    if (video_dst_file) {
        fclose(video_dst_file);
    }
    if (audio_dst_file) {
        fclose(audio_dst_file);
    }
    av_frame_free(&frame);
}

int AVFormat::open_codec_context(int *stream_idx,
                               AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx,
                               AVMediaType type){
    int ret;
    int stream_index;
    AVStream *st;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        LOGE("Could not find %s stream in input file.\n", av_get_media_type_string(type));
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];
        
        // find decoder for the stream
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            LOGE("Failed to find %s codec.\n", av_get_media_type_string(type));
            return AVERROR(EINVAL);
        } 
        
        // allocate a codec context for the decoder
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) {
            LOGE("Failed to allocate the %s codec context.\n", av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }
        
        // copy codec parameters from input stream to output codec context
        ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar) < 0;
        if (ret) {
            LOGE("Failed to copy %s codec parameters to decoder context.\n", av_get_media_type_string(type));
            return ret;
        }

        // init the decoders
        ret = avcodec_open2(*dec_ctx, dec, &opts) < 0;
        if (ret< 0) {
            LOGE("Failed to open %s codec.\n", av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }
    return 0;
}

int AVFormat::decode_packet(AVCodecContext *dec, const AVPacket *pkt) {
    int ret = 0;

    // submit the packet to the decoder
    ret = avcodec_send_packet(dec, pkt);
    if (ret < 0) {
        LOGE("Error submitting a packet for decoding (%s)\n", av_err2str(ret));
        return ret;
    }

    // get all the available frames from the decoder
    while (ret > 0) {
        ret = avcodec_receive_frame(dec, frame);
        // those two return values are special and mean there is no output
        // frame available, but there were no errors during decoding
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) {
            return 0;
        }

        // write the frame data to output file
        if (dec->codec->type == AVMEDIA_TYPE_VIDEO) {
            ret = output_audio_frame(frame);
        }

        av_frame_unref(frame);

        if (ret < 0) {
            return ret;
        }
    }
    return 0;
}

int AVFormat::output_audio_frame(AVFrame *frame) {
    size_t unpadded_linesize = static_cast<size_t>(frame->nb_samples * av_get_bytes_per_sample(
            static_cast<AVSampleFormat>(frame->format)));
    LOGD("audio_frame n:%d nb_samples:%d pts:%s\n",
           audio_frame_count++, frame->nb_samples,
           av_ts2timestr(frame->pts, &audio_dec_ctx->time_base));

    /* Write the raw audio data samples of the first plane. This works
     * fine for packed formats (e.g. AV_SAMPLE_FMT_S16). However,
     * most audio decoders output planar audio, which uses a separate
     * plane of audio samples for each channel (e.g. AV_SAMPLE_FMT_S16P).
     * In other words, this code will write only the first audio channel
     * in these cases.
     * You should use libswresample or libavfilter to convert the frame
     * to packed data. */
    fwrite(frame->extended_data[0], 1, unpadded_linesize, audio_dst_file);

    return 0;
}
