package com.hxj.ffmpegplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import java.io.File;

public class MainActivity extends AppCompatActivity {

    public static final String TAG = "MainActivity";

    static {
        System.loadLibrary("ffmpegplayer");
    }

    private TextView mTvMessage;

    private Button mBtnPrepare;

    public native String messageFromJNI();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mTvMessage = findViewById(R.id.tv_message);
        mBtnPrepare = findViewById(R.id.btn_prepare);
        mTvMessage.setText("本地函数调用返回结果: " + messageFromJNI());

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
                    ffmpegPrepare();
                }
            }
        });
    }

    private void ffmpegPrepare() {
        FFmpegPlayer fmpegPlayer = FFmpegPlayer.getInstance();
        fmpegPlayer.setOnPreparedListener(new FFmpegPlayer.OnPreparedListener() {

            @Override
            public void onPrepared() {
                Log.i(TAG, "onPrepared 开始解析: ");
            }
        });

        File file = new File(Environment.getExternalStorageDirectory(), "chengdu.mp4");
        fmpegPlayer.setDataSource(file.getAbsolutePath());
        fmpegPlayer.prepare();
    }
}
