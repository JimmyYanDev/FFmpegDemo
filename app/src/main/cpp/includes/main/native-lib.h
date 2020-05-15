//
// Created by yanqinming on 2020/5/14.
//

#ifndef FFMPEGDEMO_NATIVE_LIB_H
#define FFMPEGDEMO_NATIVE_LIB_H
#include <jni.h>
#include <string>
extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavdevice/avdevice.h"
    #include "libavfilter/avfilter.h"
    #include "libavutil/avutil.h"
    #include "libpostproc/postprocess.h"
    #include "libswresample/swresample.h"
    #include "libswscale/swscale.h"
};


extern "C" JNIEXPORT jstring JNICALL
Java_com_qmyan_ffmpegdemo_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */);
#endif //FFMPEGDEMO_NATIVE_LIB_H
