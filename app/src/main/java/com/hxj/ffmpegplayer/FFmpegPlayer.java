package com.hxj.ffmpegplayer;

import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class FFmpegPlayer implements SurfaceHolder.Callback {

    public static final String TAG = "FFmpegPlayer";

    static {
        System.loadLibrary("ffmpegplayer");
    }

    private static FFmpegPlayer instance;

    private OnPreparedListener mOnPreparedListener;
    private String mDataSource;

    private SurfaceHolder mSurfaceHolder;

    private OnProgressListener mOnProgressListener;

    private OnPlayerListener mOnPlayerListener;

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

    /**
     * 获取视频总时长.
     * @return
     */
    public int getDuration() {
        return getDurationNative();
    }

    public void stop() {
        stopNative();
    }

    public void release() {
        releaseNative();
    }

    public void onPrepared(int status) {
        Log.i(TAG, "onPrepared status: " + status);
        if (mOnPreparedListener != null) {
            mOnPreparedListener.onPrepared(status);
        }
    }

    public void onPlayer(int status) {
        if (mOnPlayerListener != null) {
            mOnPlayerListener.onError(status);
        }
    }

    /**
     *  通知进度改变.
     * @param progress
     */
    public void onProgress(int progress) {
        if (mOnProgressListener != null) {
            mOnProgressListener.onProgress(progress);
        }
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        if (mSurfaceHolder != null) {
            mSurfaceHolder.removeCallback(this);
        }

        mSurfaceHolder = surfaceView.getHolder();
        mSurfaceHolder.addCallback(this);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        setSurfaceNative(holder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public void seek(int playProgress) {
        seekNative(playProgress);
    }

    public interface OnPreparedListener {

        void onPrepared(int status);
    }

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.mOnPreparedListener = onPreparedListener;
    }

    public interface OnProgressListener {

        void onProgress(int progress);
    }

    public void setOnProgressListener(OnProgressListener l) {
        this.mOnProgressListener = l;
    }

    public interface OnPlayerListener {

        void onError(int status);
    }

    public void setOnPlayerListenner(OnPlayerListener l) {
        this.mOnPlayerListener = l;
    }

    private native void startNative();

    private native void stopNative();

    private native void releaseNative();

    private native void prepareNative(String mDataSource);

    private native void setSurfaceNative(Surface surface);

    private native int getDurationNative();

    private native void seekNative(int playProgress);
}
