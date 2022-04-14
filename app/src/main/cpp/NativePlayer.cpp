

#include "NativePlayer.h"


/**
 * 构造函数
 * @param data_source
 */
NativePlayer::NativePlayer(const char *data_source, JNICallbackHelper *pHelper) {
    // 深拷贝，免得变量释放后导致错误，长度+1 ==> 处理 \0
    this->data_source = new char[strlen(data_source) + 1];
    // 把源 Copy给成员
    strcpy(this->data_source, data_source);
    this->helper = pHelper;
}

/**
 * 析构函数
 */
NativePlayer::~NativePlayer() {
    if (data_source) {
        delete data_source;
    }

    if (helper) {
        delete helper;
    }

}

// 子线程的函数指针(prepare的)
void *task_prepare(void *args) {
    // 传递了this进来，所以args就是NativePlayer
    // 为什么传递进来？ 因为该方法拿不到this，和NativePlayer是没联系的
    // 用友元也ok
    NativePlayer *player = static_cast<NativePlayer *>(args);

    player->prepare_();

    return nullptr; // 一定要返回
}


// 子线程调用的,真正对ffmpeg-api的调用
void NativePlayer::prepare_() {

    // TODO 第一步：打开媒体地址（文件路径， 直播地址rtmp）
    formatContext = avformat_alloc_context();


    AVDictionary *dictionary = nullptr;
    av_dict_set(&dictionary, "timeout", "5000000", 0); // 单位微妙

    int result = 0;
    // TODO 输入流
    /**
     * 1，AVFormatContext 上下文
     * 2，data_source 路径
     * 3，AVInputFormat *fmt  Mac、Windows 摄像头、麦克风， 我们目前安卓用不到
     * 4，各种设置：例如：Http 连接超时， 打开rtmp的超时  AVDictionary **options
     */
    result = avformat_open_input(&formatContext, this->data_source, nullptr, &dictionary);

    // 释放字典
    av_dict_free(&dictionary);

    // 0 on success
    if (result) {
        // 回调JAVA，反馈错误 JNI反射
        if (helper) {
            helper->errorCallBack(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        }

        return;
    }

    // TODO 第二步：查找 视/音频流 信息 保存在上下文 nb_streams 中
    result = avformat_find_stream_info(formatContext, nullptr);

    // >=0 if OK, AVERROR_xxx on error
    if (result < 0) {
        if (helper) {
            helper->errorCallBack(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        }
        return;
    }

    // TODO 第三步：遍历每个流，根据流的信息做对应操作
    for (int i = 0; i < formatContext->nb_streams; ++i) {

        // TODO 第四步：获取媒体流（视频，音频）
        AVStream *stream = formatContext->streams[i];

        // TODO 第五步：从上面的流中 获取 编码解码的【参数】
        AVCodecParameters *parameters = stream->codecpar;

        // TODO 第六步：（根据上面的【参数】）获取编解码器
        AVCodec *codec = avcodec_find_decoder(parameters->codec_id);
        if (!codec) {
            if (helper) {
                helper->errorCallBack(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            }
            return;
        }


        // TODO 第七步：编解码器 上下文 （这个才是真正干活的）
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            if (helper) {
                helper->errorCallBack(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            }
            return;
        }

        // TODO 第八步：他目前是一张白纸（parameters copy codecContext）
        result = avcodec_parameters_to_context(codecContext, parameters);
        // >= 0 on success,
        if (result < 0) {
            if (helper) {
                helper->errorCallBack(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            }
            return;
        }
        // TODO 第九步：打开解码器
        result = avcodec_open2(codecContext, codec, 0);
        if (result) { // 非0就是true
            if (helper) {
                helper->errorCallBack(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            }
            return;
        }

        // 从流中获取时间基
        AVRational time_base = stream->time_base;

        // TODO 第十步：从编解码器参数中，获取流的类型 codec_type  ===  音频/视频
        if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO) { // 音频
            audio_channel = new AudioChannel(i, codecContext, time_base);
        } else if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO) { // 视频

            // 虽然是视频类型，但是只有一帧封面
            if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
                continue;
            }

            AVRational fps_rational = stream->avg_frame_rate;
            int fps = av_q2d(fps_rational);

            video_channel = new VideoChannel(i, codecContext, time_base, fps);
            video_channel->setRenderCallback(renderCallback);
        }

    } // for end

    if (!audio_channel && !video_channel) {
        if (helper) {
            helper->errorCallBack(THREAD_CHILD, FFMPEG_NO_MEDIA);
        }
        return;
    }

    // 子线程，回调上层
    helper->prepareCallBack(THREAD_CHILD);
}

void NativePlayer::prepare() {
    // TODO 对音视频流 解封装, IO流操作，开启线程
    pthread_create(&pid_prepare, nullptr, task_prepare, this);
}

// 开始播放线程的函数指针
void *task_start(void *args) {
    auto *player = static_cast<NativePlayer *>(args);
    player->start_();

    return nullptr;
}


void NativePlayer::start_() {
    // 压缩数据存放到queue
    while (isPlaying) {
        // 开辟一个空间
        AVPacket *packet = av_packet_alloc();
        // 读取一个 音/视频 压缩数据包
        int ret = av_read_frame(formatContext, packet);
        // 0 if OK, < 0 on error or end of file
        if (!ret) {
            // 成功读取到
            if (video_channel && video_channel->stream_index == packet->stream_index) {
                // 代表是视频
                video_channel->packets.insertToQueue(packet);
            } else if (audio_channel && audio_channel->stream_index == packet->stream_index) {
                // 代表是音频
                 audio_channel->packets.insertToQueue(packet);
            }
        } else if (ret == AVERROR_EOF) {
            // 读到文件末尾
        } else {
            // error

            break;
        }
    } // while end
    isPlaying = false;
    video_channel->stop();
    audio_channel->stop();
}


void NativePlayer::start() {
    // 正在播放标志
    isPlaying = true;

    // 播放视频
    // 从队列 获取到音/视频包 --> 解开 --> 原始数据包（YUV格式） --> 转成RGBA格式 --> 渲染
    if (video_channel){
        video_channel->setAudioChannel(audio_channel);
        video_channel->start();
    }

    if (audio_channel) {
        audio_channel->start();
    }

    // TODO 把压缩的 音/视频包 放到阻塞队列中去
    pthread_create(&pid_start, nullptr, task_start, this);
}

void NativePlayer::setRenderCallback(RenderCallback renderCallback) {
    this->renderCallback = renderCallback;
}






