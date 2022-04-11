//
// Created by admin on 2022/4/7.
//

#ifndef MYNDK_AUDIOCHANNEL_H
#define MYNDK_AUDIOCHANNEL_H
#include "BaseChannel.h"

class AudioChannel : public BaseChannel {

public:
    AudioChannel(int stream_index, AVCodecContext *codecContext);

    virtual ~AudioChannel();

    void stop();
};




#endif //MYNDK_AUDIOCHANNEL_H
