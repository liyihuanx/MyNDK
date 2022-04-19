package com.liyihuanx.myndk;

/**
 * @author liyihuan
 * @date 2022/04/07
 * @Description
 */
public interface PlayerListener {

	void onPrepared();

	void onError(String msg);

	void onProgress(int progress);

	void onStatusChange(String msg);
}
