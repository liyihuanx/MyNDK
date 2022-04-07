package com.liyihuanx.myndk

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.widget.Toast
import com.liyihuanx.myndk.databinding.ActivityMainBinding
import java.io.File

/**
 * 用javah的方式生成C层的头文件
 * 打开java文件夹 cmd -> javah 包名.类名(com.liyihuanx.myndk.MainActivity)
 */
class MainActivity : AppCompatActivity() {

    private val simplePlayer by lazy {
        SimplePlayer()
    }

    private val dataSource = "/data/data/com.liyihuanx.myndk/demo.mp4"

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        val path = File("${Environment.getExternalStorageDirectory()}${File.separator}demo.mp4").absolutePath
        Log.d("QWER", "path $path")

        simplePlayer.setDataSource(
            dataSource
        )


        simplePlayer.setPlayerListener {
            runOnUiThread {
                Toast.makeText(this@MainActivity, "准备成功，即将开始播放", Toast.LENGTH_LONG).show()
            }
            simplePlayer.start() // 调用 C++ 开始播放
        }
    }

    override fun onResume() {
        super.onResume()
        simplePlayer.prepare()
    }
}