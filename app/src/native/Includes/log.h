#ifndef LOGCAT_TAG

#if 1 == 1
#include "android/log.h"
#define LOGCAT_TAG "EpilogLib"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOGCAT_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOGCAT_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOGCAT_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOGCAT_TAG, __VA_ARGS__)
#else
#define LOGCAT_TAG ""
#define LOGD(...) 0
#define LOGW(...) 0
#define LOGE(...) 0
#define LOGI(...) 0
#endif

#endif