package com.liyihuanx.myndk;

/**
 * @author liyihuan
 * @date 2022/04/08
 * @Description
 */
public class ConstantCode {
	// 错误代码 ================ 如下
	// 打不开视频
	// #define FFMPEG_CAN_NOT_OPEN_URL 1
	public static final int FFMPEG_CAN_NOT_OPEN_URL = 1;

	// 找不到流媒体
	// #define FFMPEG_CAN_NOT_FIND_STREAMS 2
	public static final int FFMPEG_CAN_NOT_FIND_STREAMS = 2;

	// 找不到解码器
	// #define FFMPEG_FIND_DECODER_FAIL 3
	public static final int FFMPEG_FIND_DECODER_FAIL = 3;

	// 无法根据解码器创建上下文
	// #define FFMPEG_ALLOC_CODEC_CONTEXT_FAIL 4
	public static final int FFMPEG_ALLOC_CODEC_CONTEXT_FAIL = 4;

	//  根据流信息 配置上下文参数失败
	// #define FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL 6
	public static final int FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL = 6;

	// 打开解码器失败
	// #define FFMPEG_OPEN_DECODER_FAIL 7
	public static final int FFMPEG_OPEN_DECODER_FAIL = 7;

	// 没有音视频
	// #define FFMPEG_NOMEDIA 8
	public static final int FFMPEG_NO_MEDIA = 8;

}
