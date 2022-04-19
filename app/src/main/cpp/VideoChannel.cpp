//
// Created by admin on 2022/4/7.
//

#include "VideoChannel.h"

void dropAVFrame(queue<AVFrame *> &q){
    if (!q.empty()) {
        AVFrame *frame = q.front();
        av_frame_unref(frame);
        BaseChannel::releaseAVFrame(&frame);
        q.pop();
    }
}

void dropAVPacket(queue<AVPacket *> &q){
    while (!q.empty()) {
        AVPacket *pkt = q.front();
        if (pkt->flags != AV_PKT_FLAG_KEY) { // 非关键帧，可以丢弃
            av_packet_unref(pkt);
            BaseChannel::releaseAVPacket(&pkt);
            q.pop();
        } else {
            break; // 如果是关键帧，不能丢，那就结束
        }
    }
}


VideoChannel::VideoChannel(int stream_index, AVCodecContext *codecContext, AVRational time_base,
                           int fps) : BaseChannel(
        stream_index, codecContext, time_base), fps(fps) {
    frames.setSyncCallback(dropAVFrame);
    packets.setSyncCallback(dropAVPacket);
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
    AVPacket *pkt = nullptr;
    while (isPlaying) {

        if (isPlaying && frames.size() > 50) {
            av_usleep(10 * 1000); //  10毫秒
            continue;
        }

        // 从压缩数据包取出一个
        int ret = packets.getQueueAndDel(pkt);
        // 取出来后，发现没播放了
        if (!isPlaying) {
            break;
        }
        // 取失败了，还是得继续取下一个
        // 0-false,1-true
        if (!ret) {
            continue;
        }

        // 发送给缓冲区，源码Copy一份，后续再从缓冲区拿
        ret = avcodec_send_packet(codecContext, pkt);
        // 已经在缓冲区存在一份了，可以释放掉
//        releaseAVPacket(&pkt); --> 这种释放方式只释放了pkt指向的堆空间，而pkt里的指针指向的地址没被释放

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
            if (frame) {
                av_frame_unref(frame);
                releaseAVFrame(&frame);
            }
            break; // 错误了
        }
        // 数据原始包插入队列
        frames.insertToQueue(frame);

        // packet已经被使用完了，做释放
        av_packet_unref(pkt);
        releaseAVPacket(&pkt); // 释放AVPacket * 本身的堆区空间

    } // while end

    // 在break后
    av_packet_unref(pkt);
    releaseAVPacket(&pkt);
}


void VideoChannel::video_play() {
    // 取出原始包做渲染工作
    AVFrame *frame = nullptr;
    uint8_t *dst_data[4]; // RGBA_8888 = 4bit
    int dst_line_size[4]; // 每一行的

    // 奖原始包（YUV数据）--> [libswscale] --> Android屏幕（RGBA数据）

    // 1.开辟内存 --> 大小 = width * height * 4bit
    av_image_alloc(dst_data, dst_line_size,
                   codecContext->width, codecContext->height,
                   AV_PIX_FMT_RGBA, 1);

    // 2.转换数据,先创建上下文
    SwsContext *sws_context = sws_getContext(
            codecContext->width, codecContext->height,
            codecContext->pix_fmt, // 自动获取 xxx.mp4 的像素格式  AV_PIX_FMT_YUV420P

            codecContext->width, codecContext->height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, NULL, NULL, NULL
    );

    while (isPlaying) {
        // 取出一个原始数据包
        int ret = frames.getQueueAndDel(frame);
        if (!isPlaying) {
            break; // 如果关闭了播放，跳出循环，释放资源
        }
        if (!ret) {
            continue;
        }

        // 格式转换
        sws_scale(sws_context, frame->data, frame->linesize,
                  0, codecContext->height,
                  dst_data,
                  dst_line_size);

        // 音视频同步，在播放每个frames时候，加入间隔fps来延迟

        double extra_delay = frame->repeat_pict / (2 * fps); // 在之前的编码时，加入的额外延时时间取出来（可能获取不到）
        double fps_delay = 1.0 / fps; // 根据fps得到延时时间（fps25 == 每秒25帧，计算每一帧的延时时间，0.040000）
        double real_delay = fps_delay + extra_delay; // 当前帧的延时时间  0.040000

        // 获取 音频 和 视频 的时间
        double video_time = frame->best_effort_timestamp * av_q2d(time_base);
        double audio_time = audio_channel->audio_time;

        // 判断两个时间差值
        double time_diff = video_time - audio_time;

        if (time_diff > 0) {
            // 视频比音频快，直接睡觉，等一会
            if (time_diff > 1) {
                // 相差较大
                av_usleep((real_delay * 2) * 1000000); // 转成微秒
            } else {
                av_usleep((real_delay + time_diff) * 1000000);
            }
        }
        if (time_diff < 0) {
            // 视频比音频慢，要丢包同步
            if (fabs(time_diff) <= 0.05) {
                frames.sync();
                continue;
            }
        }


        // native: ANativeWindows 做渲染工作
        // 上层: SurfaceView 做显示工作
        // 这里拿不到SurfaceView，所以回调出去
        renderCallback(dst_data[0], codecContext->width, codecContext->height, dst_line_size[0]);
        av_frame_unref(frame);
        releaseAVFrame(&frame);
    }
    av_frame_unref(frame);
    releaseAVFrame(&frame); // 出现错误，所退出的循环，都要释放frame

    isPlaying = false;
    av_free(&dst_data[0]);
    sws_freeContext(sws_context); // free(sws_ctx);
}

void VideoChannel::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}

void VideoChannel::setAudioChannel(AudioChannel *audio_channel) {
    this->audio_channel = audio_channel;
}


