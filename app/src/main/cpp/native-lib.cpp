#include <main/AVFormat.h>
#include "main/native-lib.h"
#include "utils/FFmpegUtils.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_qmyan_ffmpegdemo_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {


    std::string hello = "FFmpeg 支持的音视频编码方式： \n";
    av_register_all();
    AVCodec *pAvCodec = av_codec_next(NULL);

    while (pAvCodec != NULL) {
        std::string temp = pAvCodec->name;
        switch (pAvCodec->type) {
            case AVMediaType::AVMEDIA_TYPE_VIDEO: {
                hello += "[AVMEDIA_TYPE_VIDEO] " + temp + "\n";
                break;
            }
            case AVMediaType::AVMEDIA_TYPE_AUDIO: {
                hello += "[AVMEDIA_TYPE_AUDIO] " + temp + "\n";
                break;
            }
            default: {
                hello += "[AVMEDIA_TYPE_OTHER] " + temp + "\n";
                break;
            }
        }
        pAvCodec = pAvCodec->next;
    }
    return env->NewStringUTF(hello.c_str());
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_qmyan_ffmpegdemo_MainActivity_demuxing_1decoding(JNIEnv *env, jobject thiz, jstring src,
                                                          jstring audio_output_file,
                                                          jstring video_output_file) {
    const char *srcFilePath = env->GetStringUTFChars(src, 0);
    const char *audioOutputFilePath = env->GetStringUTFChars(audio_output_file, 0);
    const char *videoOutputFilePath = env->GetStringUTFChars(video_output_file, 0);
    int ret = FFmpegUtils::demuxing_decoding(srcFilePath, audioOutputFilePath,
                                                                videoOutputFilePath);
    if (ret >= 0) {
        return jboolean(true);
    } else {
        return jboolean(false);
    }
//    return static_cast<jboolean>(AVFormat().demuxing_decoding(srcFilePath, audioOutputFilePath, videoOutputFilePath));
//    return static_cast<jboolean>(demuxing_decoding(srcFilePath, audioOutputFilePath, videoOutputFilePath));
}

extern "C" jboolean Java_com_qmyan_ffmpegdemo_MainActivity_remuxing(JNIEnv *env, jobject thiz,
                                                                    jstring origin_file,
                                                                    jstring target_file) {
    const char *inFileName = env->GetStringUTFChars(origin_file, 0);
    const char *outFileName = env->GetStringUTFChars(target_file, 0);

    return 0;
}
