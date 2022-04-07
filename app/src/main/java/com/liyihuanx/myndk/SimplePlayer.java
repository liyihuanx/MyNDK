package com.liyihuanx.myndk;

/**
 * @author liyihuan
 * @date 2022/04/07
 * @Description
 */
public class SimplePlayer {

	static {
		System.loadLibrary("myndk");
	}

	// 播放地址
	private String dataSource;


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


	/**
	 *
	 */
	public PlayerListener playerListener;

	public void setPlayerListener(PlayerListener playerListener) {
		this.playerListener = playerListener;
	}


	/**
	 * native层方法
	 */
	public native void nativePrepare(String dataSource);
	public native void nativeStart();
	public native void nativeStop();
	public native void nativePause();
	public native void nativeResume();

}
