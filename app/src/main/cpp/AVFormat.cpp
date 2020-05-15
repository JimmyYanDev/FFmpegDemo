//
// Created by yanqinming on 2020/5/15.
//

#include "includes/main/AVFormat.h"



int AVFormat::get_format_from_sample_fmt(const char **fmt,
                                      enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
            { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
            { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
            { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
            { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
            { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
    };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr,
            "sample format %s is not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return -1;
}

void AVFormat::demuxing_decoding(const char *srcfile, const char *audio_output_file,
                                 const char *video_output_file) {
    int got_frame;
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
        AVPacket orig_pkt = pkt;
        if (pkt.stream_index == audio_stream_idx) {
            do {
                LOGD("read frames from the file...start");
                int ret = decode_packet(&got_frame, 0);
                if (ret < 0) {
                    break;
                }
                pkt.data += ret;
                pkt.size -= ret;
                LOGD("read frames from the file...pkt.size-%d", pkt.size);
            } while (pkt.size > 0);
        } else {
            continue;
        }
        av_packet_unref(&orig_pkt);
    }

    // flush the decoders
    pkt.data = NULL;
    pkt.size = 0;
    do {
        LOGD("flush the decoders...");
        decode_packet(&got_frame, 1);
    }while (got_frame);
    LOGD("Demuxing succeeded.\n");



    if (audio_stream) {
        enum AVSampleFormat sfmt = audio_dec_ctx->sample_fmt;
        int n_channels = audio_dec_ctx->channels;
        const char *fmt;

        if (av_sample_fmt_is_planar(sfmt)) {
            const char *packed = av_get_sample_fmt_name(sfmt);
            LOGD("Warning: the sample format the decoder produced is planar "
                   "(%s). This example will output the first channel only.\n",
                   packed ? packed : "?");
            sfmt = av_get_packed_sample_fmt(sfmt);
            n_channels = 1;
        }

        if (get_format_from_sample_fmt(&fmt, sfmt) < 0){
            goto end;
        }

        LOGD("Play the output audio file with the command:\n"
               "ffplay -f %s -ac %d -ar %d %s\n",
               fmt, n_channels, audio_dec_ctx->sample_rate,
               audio_output_file);
    }

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
        av_dict_set(&opts, "refcounted_frames", 0, 0);
        ret = avcodec_open2(*dec_ctx, dec, &opts) < 0;
        if (ret< 0) {
            LOGE("Failed to open %s codec.\n", av_get_media_type_string(type));
            return ret;
        }
        *stream_idx = stream_index;
    }
    return 0;
}

int AVFormat::decode_packet(int *got_frame, int cached) {

    int ret = 0;
    int decoded = pkt.size;

    *got_frame = 0;

    if (pkt.stream_index == audio_stream_idx) {
        /* decode audio frame */
        ret = avcodec_decode_audio4(audio_dec_ctx, frame, got_frame, &pkt);
        if (ret < 0) {
            LOGE("Error decoding audio frame (%s)\n", av_err2str(ret));
            return ret;
        }
        /* Some audio decoders decode only part of the packet, and have to be
         * called again with the remainder of the packet data.
         * Sample: fate-suite/lossless-audio/luckynight-partial.shn
         * Also, some decoders might over-read the packet. */
        decoded = FFMIN(ret, pkt.size);

        if (*got_frame) {
            output_audio_frame(frame);
        }
    }

    /* If we use frame reference counting, we own the data and need
     * to de-reference it when we don't use it anymore */
    if (*got_frame && 0)
    av_frame_unref(frame);

    return decoded;
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
