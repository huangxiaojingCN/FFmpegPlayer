package com.hxj.ffmpegplayer;

import android.util.Log;

public class FFmpegPlayer {

    public static final String TAG = "FFmpegPlayer";

    static {
        System.loadLibrary("ffmpegplayer");
    }

    private static FFmpegPlayer instance;

    private OnPreparedListener mOnPreparedListener;
    private String mDataSource;

    public static FFmpegPlayer getInstance() {
        if (instance == null) {
            synchronized (FFmpegPlayer.class) {
                if (instance == null) {
                    instance = new FFmpegPlayer();
                }
            }
        }
        return instance;
    }

    public void setDataSource(String dataSource) {
        this.mDataSource = dataSource;
    }

    public void prepare() {
        prepareNative(this.mDataSource);
    }

    public void start() {
        startNative();
    }

    public void stop() {
        stopNative();
    }

    public void release() {
        releaseNative();
    }

    private native void startNative();

    private native void stopNative();

    private native void releaseNative();

    private native void prepareNative(String mDataSource);

    public void onPrepared(int status) {
        Log.i(TAG, "onPrepared status: " + status);
        if (mOnPreparedListener != null) {
            mOnPreparedListener.onPrepared(status);
        }
    }

    public interface OnPreparedListener {

        void onPrepared(int status);
    }

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.mOnPreparedListener = onPreparedListener;
    }
}
