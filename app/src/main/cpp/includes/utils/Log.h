//
// Created by yanqinming on 2020/5/15.
//

#ifndef FFMPEGDEMO_LOG_H
#define FFMPEGDEMO_LOG_H

#include <android/log.h>
#define TAG "qmyan"
#define LOGD(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, fmt, ##__VA_ARGS__)

#endif //FFMPEGDEMO_LOG_H
