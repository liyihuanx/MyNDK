package com.liyihuanx.myndk

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.TextView
import com.liyihuanx.myndk.databinding.ActivityMainBinding

/**
 * 用javah的方式生成C层的头文件
 * 打开java文件夹 cmd -> javah 包名.类名(com.liyihuanx.myndk.MainActivity)
 */
class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

    }



    companion object {
        // Used to load the 'myndk' library on application startup.
        init {
            System.loadLibrary("myndk")
        }
    }
}