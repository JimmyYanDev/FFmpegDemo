#include <main/AVFormat.h>
#include "main/native-lib.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_qmyan_ffmpegdemo_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {


    std::string hello = "FFmpeg 支持的音视频编码方式： \n";
    av_register_all();
    AVCodec *pAvCodec = av_codec_next(NULL);

    while(pAvCodec != NULL) {
        std::string temp = pAvCodec->name;
        switch (pAvCodec->type) {
            case AVMediaType::AVMEDIA_TYPE_VIDEO: {
                hello += "[AVMEDIA_TYPE_VIDEO] " +  temp + "\n";
                break;
            }
            case AVMediaType::AVMEDIA_TYPE_AUDIO: {
                hello += "[AVMEDIA_TYPE_AUDIO] " +  temp + "\n";
                break;
            }
            default: {
                hello += "[AVMEDIA_TYPE_OTHER] " +  temp + "\n";
                break;
            }
        }
        pAvCodec = pAvCodec->next;
    }
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_com_qmyan_ffmpegdemo_MainActivity_demuxing_1decoding(JNIEnv *env, jobject thiz, jstring src,
                                                          jstring audio_output_file,
                                                          jstring video_output_file) {
    const char *csrc = env->GetStringUTFChars(src, 0);
    const char *caudio_output_file = env->GetStringUTFChars(audio_output_file, 0);
    const char *cvideo_output_file = env->GetStringUTFChars(video_output_file, 0);
    AVFormat().demuxing_decoding(csrc, caudio_output_file, cvideo_output_file);
}
