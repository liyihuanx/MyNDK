

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
        LOGD("输入流开启失败：%d", result);
        return;
    }

    // TODO 第二步：查找 视/音频流 信息 保存在上下文 nb_streams 中
    result = avformat_find_stream_info(formatContext, nullptr);

    // >=0 if OK, AVERROR_xxx on error
    if (result < 0) {
        LOGD("查找 视/音频流 信息 失败：%d", result);
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

        // TODO 第七步：编解码器 上下文 （这个才是真正干活的）
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            LOGD("编解码器 上下文创建创建 失败");
            return;
        }

        // TODO 第八步：他目前是一张白纸（parameters copy codecContext）
        result = avcodec_parameters_to_context(codecContext, parameters);
        // >= 0 on success,
        if (result < 0) {
            LOGD("编解码器赋值 失败：%d", result);
            return;
        }
        // TODO 第九步：打开解码器
        result = avcodec_open2(codecContext, codec, 0);
        if (result) { // 非0就是true
            LOGD("打开解码器 失败：%d", result);
            return;
        }

        // TODO 第十步：从编解码器参数中，获取流的类型 codec_type  ===  音频/视频
        if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO) { // 音频
            audio_channel = new AudioChannel();
        } else if (parameters->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO) { // 视频
            video_channel = new VideoChannel();
        }

    } // for end

    if (!audio_channel && !video_channel) {
        LOGD("最后的失败");
        return;
    }

    // 子线程，回调上层
    helper->prepareCallBack(THREAD_CHILD);
}

void NativePlayer::prepare() {
    // TODO 对音视频流 解封装, IO流操作，开启线程
    pthread_create(&pid_prepare, nullptr, task_prepare, this);
}



