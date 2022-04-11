//
// Created by admin on 2022/4/7.
//

#include "VideoChannel.h"

VideoChannel::VideoChannel(int stream_index, AVCodecContext *codecContext) : BaseChannel(
        stream_index, codecContext) {

}

VideoChannel::~VideoChannel() {

}

void VideoChannel::stop() {

}

void *task_video_decode(void *args) {
    auto video_channel = static_cast<VideoChannel *> (args);
    video_channel->video_decode();
    return nullptr;
}

void *task_video_play(void *args) {
    auto video_channel = static_cast<VideoChannel *> (args);
    video_channel->video_play();
    return nullptr;
}

void VideoChannel::start() {
    isPlaying = true;

    // 队列开始工作了
    packets.setWork(1);
    frames.setWork(1);

    // 线程一: 解码
    pthread_create(&pid_video_decode, nullptr, task_video_decode, this);
    // 线程二: 播放
    pthread_create(&pid_video_play, nullptr, task_video_play, this);
}

void VideoChannel::video_decode() {
    AVPacket *pkt = 0;
    while (isPlaying) {
        // 从压缩数据包取出一个
        int ret = packets.getQueueAndDel(pkt);
        // 取出来后，发现没播放了
        if (isPlaying) {
            break;
        }
        // 取失败了，还是得继续取下一个
        // 0-false,1-true
        if (!ret) {
            continue;
        }

        // 发送给缓冲区，再从缓冲区拿
        ret = avcodec_send_packet(codecContext, pkt);
        // 已经在缓冲区存在一份了，可以释放掉
        releaseAVPacket(&pkt);
        if (ret) {
            break;
        }

        // 下面是从 缓冲区 获取 原始包
        // 申请一块空间
        AVFrame *frame = av_frame_alloc();
        // 接收一个解压后的数据原始包
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            // B帧  B帧参考前面成功  B帧参考后面失败   可能是P帧没有出来，再拿一次就行了
            continue;
        } else if (ret != 0) {
            break; // 错误了
        }
        // 数据原始包插入队列
        frames.insertToQueue(frame);

    } // while end

    // 在break后
    releaseAVPacket(&pkt);
}

void VideoChannel::video_play() {
    // 取出原始包做渲染工作


}


