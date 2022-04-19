package com.liyihuanx.myndk

import android.annotation.SuppressLint
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Environment
import android.util.Log
import android.view.View
import android.widget.SeekBar
import android.widget.Toast
import com.liyihuanx.myndk.databinding.ActivityMainBinding
import java.io.File

/**
 * 用javah的方式生成C层的头文件
 * 打开java文件夹 cmd -> javah 包名.类名(com.liyihuanx.myndk.MainActivity)
 */
class MainActivity : AppCompatActivity(), SeekBar.OnSeekBarChangeListener {

    private val simplePlayer by lazy {
        SimplePlayer()
    }

    private val dataSource = "/data/data/com.liyihuanx.myndk/demo.mp4"

    private lateinit var binding: ActivityMainBinding

    // 用户是否拖拽了 拖动条，（默认是没有拖动false）
    private var isTouch = false

    // 获取native层的总时长
    private var duration = 0

    private var isPlaying = true;

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        val path =
            File("${Environment.getExternalStorageDirectory()}${File.separator}demo.mp4").absolutePath
        Log.d("QWER", "path $path")


        binding.seekBar.setOnSeekBarChangeListener(this)


        simplePlayer.setSurfaceView(binding.sampleSv);
        simplePlayer.setDataSource(dataSource)

        simplePlayer.setPlayerListener(object : PlayerListener {
            @SuppressLint("SetTextI18n")
            override fun onPrepared() {
                duration = simplePlayer.duration
                runOnUiThread {
                    Toast.makeText(this@MainActivity, "准备成功，即将开始播放", Toast.LENGTH_LONG).show()
                    if (duration != 0) {
                        binding.tvTime.text =
                            "00:00/${getMinutes(duration)}:${getSeconds(duration)}";
                        binding.tvTime.visibility = View.VISIBLE // 显示
                        binding.seekBar.visibility = View.VISIBLE // 显示
                    }

                }


                // TODO 第七节课增加 1.1 拖动条默认隐藏，如果播放视频有总时长，就显示所以拖动条控件
                // 得到视频总时长： 直播：duration=0，  非直播-视频：duration=有值的

                simplePlayer.start() // 调用 C++ 开始播放
            }

            override fun onError(msg: String) {
                runOnUiThread {
                    Toast.makeText(this@MainActivity, "出错了: $msg", Toast.LENGTH_LONG).show()
                }
            }

            @SuppressLint("SetTextI18n")
            override fun onProgress(progress: Int) {
                if (!isTouch) {
                    runOnUiThread {
                        if (duration != 0) {
                            binding.tvTime.text =
                                "${getMinutes(progress)}:${getSeconds(progress)}/${getMinutes(duration)}:${getSeconds(duration)}"

                            binding.seekBar.progress = progress * 100 / duration
                        }
                    }
                }

            }

            override fun onStatusChange(msg: String) {
                Toast.makeText(this@MainActivity, msg, Toast.LENGTH_LONG).show()
            }
        })


        binding.sampleSv.setOnClickListener {
            if (isPlaying) {
                isPlaying = false
                simplePlayer.pause()
            } else {
                isPlaying = true
                simplePlayer.resume()
            }
        }
    }

    override fun onResume() {
        super.onResume()
        simplePlayer.prepare()
    }

    override fun onStop() {
        super.onStop()
        simplePlayer.stop()
    }

    override fun onDestroy() {
        super.onDestroy()
        simplePlayer.destroy()
    }



    // 给我一个duration，转换成xxx分钟
    private fun getMinutes(duration: Int): String {
        val minutes = duration / 60
        return if (minutes <= 9) {
            "0$minutes"
        } else "" + minutes
    }

    // 给我一个duration，转换成xxx秒
    private fun getSeconds(duration: Int): String {
        val seconds = duration % 60
        return if (seconds <= 9) {
            "0$seconds"
        } else "" + seconds
    }

    /**
     * progress : 当前进度
     * changeFromUser : 是否用户拖拽导致的改变
     */
    @SuppressLint("SetTextI18n")
    override fun onProgressChanged(seekBar: SeekBar?, progress: Int, changeFromUser: Boolean) {
        if (changeFromUser) {
            binding.tvTime.text =
                "${getMinutes(progress * duration / 100)}:${getSeconds(progress * duration / 100)}" +
                        "/${getMinutes(duration)}:${getSeconds(duration)}"
        }
    }

    override fun onStartTrackingTouch(seekBar: SeekBar) {
        isTouch = true
    }

    override fun onStopTrackingTouch(seekBar: SeekBar) {
        isTouch = false

        val seekBarProgress = seekBar.progress // 获取当前seekbar当前进度

        // SeekBar 1~100  -- 转换 -->  C++播放的时间（61.546565）
        val playProgress = seekBarProgress * duration / 100

        simplePlayer.seek(playProgress)

    }
}