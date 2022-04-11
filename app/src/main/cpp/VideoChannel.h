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

class VideoChannel : public BaseChannel {

private:
    pthread_t pid_video_decode;
    pthread_t pid_video_play;

public:
    VideoChannel(int stream_index, AVCodecContext *codecContext);

    ~VideoChannel();

    void stop();

    void start();


    void video_decode();

    void video_play();
};

#endif //MYNDK_VIDEOCHANNEL_H
