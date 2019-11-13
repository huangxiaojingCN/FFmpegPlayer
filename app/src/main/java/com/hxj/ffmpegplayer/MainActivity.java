package com.hxj.ffmpegplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Environment;
import android.os.Handler;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    public static final String TAG = "MainActivity";

    static {
        System.loadLibrary("ffmpegplayer");
    }

    private TextView mTvMessage;

    private Button mBtnPrepare;

    private SurfaceView mSurfaceView;

    private TextView mTvTime;

    private SeekBar mSeekBar;

    private FFmpegPlayer fmpegPlayer;
    private int mDuration;
    private boolean isTouch;
    private boolean isSeek;

    public native String messageFromJNI();

    @Override
    protected void onCreate(final Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mTvMessage = findViewById(R.id.tv_message);
        mBtnPrepare = findViewById(R.id.btn_prepare);
        mSurfaceView = findViewById(R.id.surface_view);
        mSeekBar = findViewById(R.id.seekbar);
        mTvTime = findViewById(R.id.tv_time);

        mTvMessage.setText("ffmpeg版本号：" + messageFromJNI());

        fmpegPlayer = FFmpegPlayer.getInstance();
        fmpegPlayer.setSurfaceView(mSurfaceView);
        File file = new File(Environment.getExternalStorageDirectory(), "gaoxiao.mp4");
        fmpegPlayer.setDataSource(file.getAbsolutePath());

        // 等待解析结果.
        fmpegPlayer.setOnPreparedListener(new FFmpegPlayer.OnPreparedListener() {

            @Override
            public void onPrepared(final int status) {
                Log.i(TAG, "onPrepared 开始解析: " + Thread.currentThread().getName());
                if (status == 0) { // 解析完成,开始播放
                    fmpegPlayer.start();
                }
                handler.post(new Runnable() {

                    @Override
                    public void run() {
                        if (status == 0) {
                            mDuration = fmpegPlayer.getDuration();

                            mTvTime.setText("00:00/" + getMinutes(mDuration) + ":" + getSeconds(mDuration));
                            Toast.makeText(MainActivity.this, "开始播放: ", Toast.LENGTH_SHORT).show();
                        } else {
                            Toast.makeText(MainActivity.this, "开始解析数据异常: ", Toast.LENGTH_SHORT).show();
                        }
                    }
                });
            }
        });

        fmpegPlayer.setOnProgressListener(new FFmpegPlayer.OnProgressListener() {

            @Override
            public void onProgress(final int progress) {
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        if (!isTouch) {
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    if (mDuration != 0) {
                                        mTvTime.setText(getMinutes(progress) + ":" + getSeconds(progress) + "/" + getMinutes(mDuration) + ":" + getSeconds(mDuration));
                                        mSeekBar.setProgress(progress * 100 / mDuration);
                                    }
                                }
                            });
                        }
                    }
                });
            }
        });

        fmpegPlayer.setOnPlayerListenner(new FFmpegPlayer.OnPlayerListener() {
            @Override
            public void onError(int status) {
                if (status == -1) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(MainActivity.this, "播放失败", Toast.LENGTH_SHORT).show();
                        }
                    });
                }
            }
        });

        // 播放.
        mBtnPrepare.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.READ_EXTERNAL_STORAGE) !=
                        PackageManager.PERMISSION_GRANTED) {
                    if (ActivityCompat.shouldShowRequestPermissionRationale(MainActivity.this, Manifest.permission.READ_EXTERNAL_STORAGE)) {
                        //
                    } else {
                        ActivityCompat.requestPermissions(MainActivity.this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, 100);
                    }
                } else {
                    fmpegPlayer.prepare();
                }
            }
        });

        mSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    mTvTime.setText(getMinutes(progress * mDuration /100) + ":"
                    + getSeconds(progress * mDuration / 100) + "/" +
                            getMinutes(mDuration) + ":" + getSeconds(mDuration));
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isTouch = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                isTouch = false;
                // 获取当前的进度.
                int progress = seekBar.getProgress();
                Log.i(TAG, "onStopTrackingTouch mDuration: " + mDuration);

                int playProgress = progress * mDuration / 100;
                Log.i(TAG, "onStopTrackingTouch: playProgress: " + playProgress);

                // 调节进度.
                fmpegPlayer.seek(playProgress);
            }
        });
    }

    private String getMinutes(int mDuration) {
        String result = "";
        int minute = mDuration / 60;
        if (minute < 10) {
            result = "0" + minute;
        } else {
            result += minute;
        }

        return result;
    }

    private String getSeconds(int mDuration) {
        String result = "";

        int second = mDuration % 60;
        if (second < 10) {
            result = "0" + second;
        } else {
            result += second;
        }

        return result;
    }

    private Handler handler = new Handler();
}
