package com.liyihuanx.myndk;


import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import androidx.annotation.NonNull;

/**
 * @author liyihuan
 * @date 2022/04/07
 * @Description
 */
public class SimplePlayer implements SurfaceHolder.Callback {

	static {
		System.loadLibrary("myndk");
	}

	// 播放地址
	private String dataSource;

	private SurfaceHolder surfaceHolder;


	public void setDataSource(String dataSource){
		this.dataSource = dataSource;
	}

	public void prepare() {
		nativePrepare(dataSource);
	}

	public void start() {
		nativeStart();
	}

	public void stop() {
		nativeStop();
	}

	public void pause() {
		nativePause();
	}

	public void resume() {
		nativeResume();
	}


	/**
	 * 给jni反射调用的
	 * native层准备完成，调用此方法
	 * 再通过Listener回调出去
	 */
	public void onPrepared() {
		if (playerListener != null) {
			playerListener.onPrepared();
		}
	}

	public void onError(int errorCode) {
		if (null != this.playerListener) {
			String msg = null;
			switch (errorCode) {
				case ConstantCode.FFMPEG_CAN_NOT_OPEN_URL:
					msg = "打不开视频";
					break;
				case ConstantCode.FFMPEG_CAN_NOT_FIND_STREAMS:
					msg = "找不到流媒体";
					break;
				case ConstantCode.FFMPEG_FIND_DECODER_FAIL:
					msg = "找不到解码器";
					break;
				case ConstantCode.FFMPEG_ALLOC_CODEC_CONTEXT_FAIL:
					msg = "无法根据解码器创建上下文";
					break;
				case ConstantCode.FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL:
					msg = "根据流信息 配置上下文参数失败";
					break;
				case ConstantCode.FFMPEG_OPEN_DECODER_FAIL:
					msg = "打开解码器失败";
					break;
				case ConstantCode.FFMPEG_NO_MEDIA:
					msg = "没有音视频";
					break;
			}
			playerListener.onError(msg);
		}
	}


	/**
	 *
	 */
	public PlayerListener playerListener;

	public void setPlayerListener(PlayerListener playerListener) {
		this.playerListener = playerListener;
	}




	public void setSurfaceView(SurfaceView surfaceView) {
		if (this.surfaceHolder != null) {
			surfaceHolder.removeCallback(this); // 清除上一次的
		}
		surfaceHolder = surfaceView.getHolder();
		surfaceHolder.addCallback(this); // 监听
	}


	@Override
	public void surfaceCreated(@NonNull SurfaceHolder surfaceHolder) {

	}

	@Override
	public void surfaceChanged(@NonNull SurfaceHolder surfaceHolder, int i, int i1, int i2) {
		setSurfaceNative(surfaceHolder.getSurface());
	}

	@Override
	public void surfaceDestroyed(@NonNull SurfaceHolder surfaceHolder) {

	}

	/**
	 * native层方法
	 */
	public native void nativePrepare(String dataSource);
	public native void nativeStart();
	public native void nativeStop();
	public native void nativePause();
	public native void nativeResume();
	private native void setSurfaceNative(Surface surface);


}
