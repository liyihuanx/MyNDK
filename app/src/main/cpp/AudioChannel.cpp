//
// Created by admin on 2022/4/7.
//

#include "AudioChannel.h"

AudioChannel::AudioChannel(int stream_index, AVCodecContext *codecContext, AVRational time_base)
        : BaseChannel(
        stream_index, codecContext, time_base) {

    // 音频三要素
    /*
     * 1.采样率 44100 48000
     * 2.位声/采用格式大小  16bit == 2字节
     * 3.声道数 2  --- 人类就是两个耳朵
     */

    // 声道布局频道数
    out_channels = av_get_channel_layout_nb_channels(
            AV_CH_LAYOUT_STEREO); // STEREO:双声道类型 == 获取 声道数 2
    // 返回每个样本的字节数
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16); // 每个sample是16 bit == 2字节

    out_sample_rate = 44100; // 采样率

    // out_buffers_size = 176,400
    // 缓冲区大小？
    out_buffers_size = out_sample_rate * out_sample_size * out_channels; // 44100 * 2 * 2 = 176,400
    // 堆区开辟缓冲区
    out_buffers = static_cast<uint8_t *>(malloc(out_buffers_size));

    // codecContext 解码器
    swr_ctx = swr_alloc_set_opts(
            nullptr,
            AV_CH_LAYOUT_STEREO,  // 声道布局类型 双声道
            AV_SAMPLE_FMT_S16,  // 采样大小 16bit
            out_sample_rate, // 采样率  44100
            codecContext->channel_layout, // 声道布局类型
            codecContext->sample_fmt, // 采样大小
            codecContext->sample_rate,  // 采样率
            0,
            0
    );
    // 初始化 重采样上下文
    swr_init(swr_ctx);

}

AudioChannel::~AudioChannel() {

}

void AudioChannel::stop() {

}

//	SLAndroidSimpleBufferQueueItf caller,
//	void *pContext
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *args) {
    auto *audio_channel = static_cast<AudioChannel *>(args);
    int pcm_size = audio_channel->get_pcm_size();
    // 添加数据到缓冲区里面去
    (*bq)->Enqueue(
            bq,
            audio_channel->out_buffers, // PCM数据
            pcm_size); // PCM数据对应的大小，缓冲区大小怎么定义？（复杂）

}

int AudioChannel::get_pcm_size() {
    int pcm_data_size = 0;
    // 原始包
    AVFrame *frame = nullptr;
    while (isPlaying) {

        int ret = frames.getQueueAndDel(frame);

        if (!isPlaying) {
            break;
        }

        if (!ret) {
            continue;
        }

        // 重采样（将音频PCM数据转换成播放器可识别的格式）
        // 重采样的一些配置
        // 假设：来源：10个48000   ---->  目标: 11个44100（10个不够，就向上取）
        int dst_nb_samples = av_rescale_rnd(
                // 获取下一个输入样本相对于下一个输出样本将经历的延迟
                swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples,
                out_sample_rate, // 输出采样率（44100）
                frame->sample_rate, // 输入采样率（音频的输入采样率，假设48000）
                AV_ROUND_UP // 先上取 取去11个才能容纳的上
        );

        // 重采样工作函数
        // samples_per_channel 每个通道输出的样本数
        int samples_per_channel = swr_convert(
                swr_ctx,
                &out_buffers,  // 重采样后的buff
                dst_nb_samples,
                (const uint8_t **) frame->data, // 队列的AVFrame * 那的  PCM数据 未重采样的
                frame->nb_samples // 此帧描述的音频样本数（每个通道
        );

        pcm_data_size = samples_per_channel * out_sample_size *
                        out_channels; // 941通道样本数  *  2样本格式字节数  *  2声道数  =3764
        audio_time = frame->best_effort_timestamp * av_q2d(time_base); // 必须这样计算后，才能拿到真正的时间搓

        jniCallbackHelper->progressCallBack(THREAD_CHILD, audio_time);

        break;
    }

    // 采样率 和 样本数的关系？
    // 样本数 = 采样率 * 声道数 * 位声
    av_frame_unref(frame);
    releaseAVFrame(&frame); // 释放AVFrame * 本身的堆区空间

    return pcm_data_size;
}


void *task_audio_decode(void *arg) {
    AudioChannel *audio = static_cast<AudioChannel *>(arg);
    audio->audio_decode();
    return nullptr;
}

void *task_audio_play(void *arg) {
    AudioChannel *audio = static_cast<AudioChannel *>(arg);
    audio->audio_play();
    return nullptr;
}

void AudioChannel::audio_decode() {
    AVPacket *pkt = nullptr;
    while (isPlaying) {
        if (isPlaying && frames.size() > 50) {
            av_usleep(10 * 1000);
            continue;
        }

        int ret = packets.getQueueAndDel(pkt); // 阻塞式函数
        if (!isPlaying) {
            break; // 如果关闭了播放，跳出循环，releaseAVPacket(&pkt);
        }

        if (!ret) { // ret == 0
            continue; // 哪怕是没有成功，也要继续（假设：你生产太慢(压缩包加入队列)，我消费就等一下你）
        }

        // 最新的FFmpeg，和旧版本差别很大， 新版本：1.发送pkt（压缩包）给缓冲区，  2.从缓冲区拿出来（原始包）
        ret = avcodec_send_packet(codecContext, pkt);

        if (ret) {
            break; // avcodec_send_packet 出现了错误，结束循环
        }

        // 下面是从 FFmpeg缓冲区 获取 原始包
        AVFrame *frame = av_frame_alloc(); // AVFrame： 解码后的视频原始数据包
        ret = avcodec_receive_frame(codecContext, frame);
        // 音频也有帧的概念，所以获取原始包的时候，最好还是判断下【严谨性，最好是判断下】
        if (ret == AVERROR(EAGAIN)) {
            continue; // 有可能音频帧，也会获取失败，重新拿一次
        } else if (ret != 0) {
            if (frame) {
                av_frame_unref(frame);
                releaseAVFrame(&frame);
            }
            break; // 错误了
        }
        // 重要拿到了 原始包-- PCM数据
        frames.insertToQueue(frame);

        av_packet_unref(pkt);
        releaseAVPacket(&pkt);
    } // while end
    av_packet_unref(pkt);
    releaseAVPacket(&pkt);

}

void AudioChannel::audio_play() {
    // openSLES 专门接收返回值的类型
    SLresult result;

    // TODO 1.创建引擎对象并获取【引擎接口】
    // 1.1 创建对象
    result = slCreateEngine(&engineObject, 0, 0, 0, 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("创建引擎 slCreateEngine error");
        return;
    }
    // 1.2 初始化 SL_BOOLEAN_FALSE:延时等待你创建成功
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("初始化引擎失败");
        return;
    }
    // 1.3 有了引擎对象后，获取引擎接口，拿到接口才能调用引擎方法
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        LOGE("创建引擎接口失败");
        return;
    }

    // TODO 2.设置混音器

    // 2.1 创建
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, 0,
                                                 0); // 环境特效，混响特效，.. 都不需要
    if (SL_RESULT_SUCCESS != result) {
        LOGD("初始化混音器 CreateOutputMix failed");
        return;
    }
    // 2.2 初始化
    result = (*outputMixObject)->Realize(outputMixObject,
                                         SL_BOOLEAN_FALSE); // SL_BOOLEAN_FALSE:延时等待你创建成功
    if (SL_RESULT_SUCCESS != result) {
        LOGD("初始化混音器 (*outputMixObject)->Realize failed");
        return;
    }
    // 2.3 获取混音器接口，就可设置声音效果

    // TODO 3.创建播放器
    // 3.1 播放器配置信息
    // 创建buffer缓存类型的队列
    SLDataLocator_AndroidSimpleBufferQueue loc_buf_queue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 10};

    // 看做是一个bean类，存放的配置信息
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM, // PCM 数据格式
            2, // 声道数，双声道
            SL_SAMPLINGRATE_44_1, // 采样率（每秒44100个点）
            SL_PCMSAMPLEFORMAT_FIXED_16, // 每秒采样样本 存放大小 16bit
            SL_PCMSAMPLEFORMAT_FIXED_16, // 每个样本位数 16bit
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT, // 前左声道  前右声道
            SL_BYTEORDER_LITTLEENDIAN // 字节序(小端)
    };

    // 看做bean类。存放的是缓存队列和配置信息
    SLDataSource audioSrc = {&loc_buf_queue, &format_pcm};

    // 3.2 配置音轨（输出）
    // SL_DATALOCATOR_OUTPUTMIX:输出混音器类型
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    // outmix最终混音器的成果，给后面代码使用
    SLDataSink audioSnk = {&loc_outmix, NULL};
    // 需要的接口 操作队列的接口
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};

    // 3.3 创建播放器
    result = (*engineInterface)->CreateAudioPlayer(engineInterface,
                                                   &bqPlayerObject, // 参数2：播放器
                                                   &audioSrc, // 参数3：音频配置信息
                                                   &audioSnk, // 参数4：混音器
                                                   1, // 参数5：开放的参数的个数
                                                   ids,  // 参数6：代表我们需要 Buff
                                                   req // 参数7：代表我们上面的Buff 需要开放出去
    );

    if (SL_RESULT_SUCCESS != result) {
        LOGD("创建播放器 CreateAudioPlayer failed!");
        return;
    }

    // 3.4 初始化
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        LOGD("实例化播放器 CreateAudioPlayer failed!");
        return;
    }
    LOGD("创建播放器 CreateAudioPlayer success!");


    // 3.5 获取播放器接口
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY,
                                             &bqPlayerPlayInterface); // SL_IID_PLAY:播放接口 == iplayer
    if (SL_RESULT_SUCCESS != result) {
        LOGD("获取播放接口 GetInterface SL_IID_PLAY failed!");
        return;
    }
    LOGI("3、创建播放器 Success");

    // TODO 4.设置回调函数
    // 获取播放器队列接口
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                             &bqPlayerBufferQueue);
    if (result != SL_RESULT_SUCCESS) {
        LOGD("获取播放队列 GetInterface SL_IID_BUFFERQUEUE failed!");
        return;
    }

    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue,  // 传入刚刚设置好的队列
                                             bqPlayerCallback,  // 回调函数
                                             this); // 给回调函数的参数


    // TODO 5.设置播放器状态为播放状态
    (*bqPlayerPlayInterface)->SetPlayState(bqPlayerPlayInterface, SL_PLAYSTATE_PLAYING);
    LOGI("5、设置播放器状态为播放状态 Success");


    // TODO 6.手动激活回调函数
    bqPlayerCallback(bqPlayerBufferQueue, this);
    LOGI("6、手动激活回调函数 Success");
}


// 开始解数据，播放
void AudioChannel::start() {
    isPlaying = true;

    packets.setWork(1);
    frames.setWork(1);

    pthread_create(&pid_audio_decode, nullptr, task_audio_decode, this);
    pthread_create(&pid_audio_play, nullptr, task_audio_play, this);

}


