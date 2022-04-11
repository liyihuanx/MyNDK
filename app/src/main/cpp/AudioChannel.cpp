//
// Created by admin on 2022/4/7.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int stream_index, AVCodecContext *codecContext) : BaseChannel(
        stream_index, codecContext) {

}

AudioChannel::~AudioChannel() {

}

void AudioChannel::stop() {

}
