package com.hxj.ffmpegplayer;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("ffmpegplayer");
    }

    private TextView mTvMessage;

    public native String messageFromJNI();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mTvMessage = findViewById(R.id.tv_message);
        mTvMessage.setText("本地函数调用返回结果: " + messageFromJNI());
    }
}
