//
// Created by yanqinming on 2020/5/14.
//

#ifndef FFMPEGDEMO_NATIVE_LIB_H
#define FFMPEGDEMO_NATIVE_LIB_H
#include <jni.h>
#include <string>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <utils/Log.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavcodec/avcodec.h>
};


extern "C" JNIEXPORT jstring JNICALL
Java_com_qmyan_ffmpegdemo_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */);

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_qmyan_ffmpegdemo_MainActivity_demuxing_1decoding(JNIEnv *env, jobject thiz, jstring src,
                                                          jstring audio_output_file,
                                                          jstring video_output_file);

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_qmyan_ffmpegdemo_MainActivity_remuxing(JNIEnv *env, jobject thiz, jstring origin_file,
                                                     jstring target_file);
#endif //FFMPEGDEMO_NATIVE_LIB_H
