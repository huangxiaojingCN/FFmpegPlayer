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

    public void onPrepared(int status) {
        Log.i(TAG, "onPrepared status: " + status);
        if (mOnPreparedListener != null) {
            mOnPreparedListener.onPrepared(status);
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

    public interface OnPreparedListener {

        void onPrepared(int status);
    }

    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        this.mOnPreparedListener = onPreparedListener;
    }

    private native void startNative();

    private native void stopNative();

    private native void releaseNative();

    private native void prepareNative(String mDataSource);

    private native void setSurfaceNative(Surface surface);
}
