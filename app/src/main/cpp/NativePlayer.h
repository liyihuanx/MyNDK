
#ifndef MYNDK_NATIVEPLAYER_H
#define MYNDK_NATIVEPLAYER_H

#include <cstring>
#include <pthread.h>
#include "AudioChannel.h"
#include "VideoChannel.h"
#include "JNICallbackHelper.h"
#include <android/log.h>
extern "C" {
#include <libavformat/avformat.h>
};
#define LOG_TAG "JNI_LOG"

//用于打印debug级别的log信息
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

//用于打印info级别的log信息
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

class NativePlayer {
private:
    char *data_source = 0;
    pthread_t pid_prepare;
    pthread_t pid_start;

    // 上下文
    AVFormatContext *formatContext = 0;
    // 音频，视频
    AudioChannel *audio_channel = 0;
    VideoChannel *video_channel = 0;
    JNICallbackHelper *helper = 0;
    // nativePlayer 使用的
    bool isPlaying = false;
    RenderCallback renderCallback;
public:
    NativePlayer(const char *data_source, JNICallbackHelper *pHelper);
    ~NativePlayer();

    void prepare();
    void prepare_();

    void start();
    void start_();

    void setRenderCallback(RenderCallback renderCallback);
};


#endif //MYNDK_NATIVEPLAYER_H
