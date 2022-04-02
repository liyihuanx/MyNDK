package com.liyihuanx.myndk;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;

import java.util.ArrayList;

public class SecondActivity extends AppCompatActivity {

	private static final String TAG = "JNI_LOG";

	static {
		System.loadLibrary("myndk");
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_second);


		findViewById(R.id.btnTest1).setOnClickListener((view) -> {
			HelloJNI();
		});

		findViewById(R.id.btnTest2).setOnClickListener((view) -> {
			String result = postString("name123");
			Log.d(TAG, "onCreate: " + result);
		});

		findViewById(R.id.btnTest3).setOnClickListener((view) -> {
			int[] arr = {1,2,3,4};

			String[] str = {"你一换","你二换","七七八八","五五六六"};

			ArrayList<String> arrayList = new ArrayList<String>();
			arrayList.add("String111");
			arrayList.add("hhhha");
			arrayList.add("cccccc");
			arrayList.get(0);
			UserBean userBean = new UserBean();
			userBean.setId(99);
			userBean.setUsername("zxcv");
			Object result = postDiffData(arr, str, arrayList, userBean);
			Log.d(TAG, "postDiffData: " + result);
		});

		findViewById(R.id.btnTest4).setOnClickListener((view) -> {
			testQuote();
		});

		findViewById(R.id.btnTest5).setOnClickListener((view) -> {
			Log.d(TAG, "dynamicRegister: " + dynamicRegister("动态注册"));
		});


		findViewById(R.id.btnTest6).setOnClickListener((view) -> {
			testNdk();
		});
	}



	public native void HelloJNI();

	/**
	 * 对String类型的简单操作
	 * @param name
	 * @return
	 */
	public native String postString(String name);

	/**
	 * 对数组，列表，对象的操作
	 * @param intArr
	 * @param str
	 * @param arrayList
	 * @param userBean
	 * @return
	 */
	public native UserBean postDiffData(int[] intArr, String[] str, ArrayList<String> arrayList, UserBean userBean);


	public native UserBean createStu();

	/**
	 *  局部引用
	 */
	public native void testQuote();


	public native String dynamicRegister(String tag);

	public native void testNdk();
}