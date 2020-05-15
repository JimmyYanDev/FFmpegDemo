//
// Created by yanqinming on 2020/5/14.
//

#ifndef FFMPEGDEMO_NATIVE_LIB_H
#define FFMPEGDEMO_NATIVE_LIB_H
#include <jni.h>
#include <string>
#include "AVFormat.h"


extern "C" JNIEXPORT jstring JNICALL
Java_com_qmyan_ffmpegdemo_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */);

extern "C"
JNIEXPORT void JNICALL
Java_com_qmyan_ffmpegdemo_MainActivity_demuxing_1decoding(JNIEnv *env, jobject thiz, jstring src,
                                                          jstring audio_output_file,
                                                          jstring video_output_file);
#endif //FFMPEGDEMO_NATIVE_LIB_H
