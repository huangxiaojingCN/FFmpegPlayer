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

    private FFmpegPlayer fmpegPlayer;

    public native String messageFromJNI();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mTvMessage = findViewById(R.id.tv_message);
        mBtnPrepare = findViewById(R.id.btn_prepare);
        mSurfaceView = findViewById(R.id.surface_view);
        mTvMessage.setText("ffmpeg版本号：" + messageFromJNI());

        fmpegPlayer = FFmpegPlayer.getInstance();
        fmpegPlayer.setSurfaceView(mSurfaceView);
        File file = new File(Environment.getExternalStorageDirectory(), "chengdu.mp4");
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
                            Toast.makeText(MainActivity.this, "开始播放: ", Toast.LENGTH_SHORT).show();
                        } else {
                            Toast.makeText(MainActivity.this, "开始解析数据异常: ", Toast.LENGTH_SHORT).show();
                        }
                    }
                });
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
    }

    private Handler handler = new Handler();
}
