package com.hxj.ffmpegplayer;

public class FFmpegPlayer {

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

    public interface OnPreparedListener {

        void onPrepared();
    }

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.mOnPreparedListener = onPreparedListener;
    }
}
