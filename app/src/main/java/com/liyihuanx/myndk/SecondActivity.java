package com.liyihuanx.myndk;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;

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
	}



	public native void HelloJNI();

	/**
	 * 对String类型的简单操作
	 * @param name
	 * @return
	 */
	public native String postString(String name);




}