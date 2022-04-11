//
// Created by admin on 2022/4/7.
//

#ifndef MYNDK_VIDEOCHANNEL_H
#define MYNDK_VIDEOCHANNEL_H

#include "BaseChannel.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
};

// 函数指针
typedef void (*RenderCallback)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel {

private:
    pthread_t pid_video_decode;
    pthread_t pid_video_play;
    RenderCallback renderCallback;

public:
    VideoChannel(int stream_index, AVCodecContext *codecContext);

    ~VideoChannel();

    void stop();

    void start();


    void video_decode();

    void video_play();

    void setRenderCallback(RenderCallback renderCallback);

};

#endif //MYNDK_VIDEOCHANNEL_H
