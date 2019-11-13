#include <jni.h>
#include "log.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

#include "FFmpegPlayer.h"
#include "JNICallbackHelper.h"
#include "pthread.h"
#include <android/native_window_jni.h>

JavaVM *javaVM = 0;
FFmpegPlayer *fFmpegPlayer = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

ANativeWindow *window = NULL;

void renderFrame(uint8_t *src_data, int width, int height, int src_lineSize);

jint JNI_OnLoad(JavaVM *vm, void *args) {
    javaVM = vm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_hxj_ffmpegplayer_MainActivity_messageFromJNI(JNIEnv *env,
        jobject instance) {

    return env->NewStringUTF(av_version_info());
}

/**
 * 数据初始化准备. 查看媒体流，根据媒体流中获得音视频流信息
 */
extern "C"
JNIEXPORT void JNICALL
 Java_com_hxj_ffmpegplayer_FFmpegPlayer_prepareNative(JNIEnv *env, jobject instance, jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, NULL);

    LOGD("dataSource: %s", dataSource);

    JNICallbackHelper *jniCallbackHelper = new JNICallbackHelper(javaVM, env, instance);
    // 创建播放器. 将 url 传递给 FFmpegPlayer
    fFmpegPlayer = new FFmpegPlayer(dataSource, jniCallbackHelper);
    fFmpegPlayer->setRenderCallback(renderFrame);
    // 开启新的线程取解析
    fFmpegPlayer->prepareAsync();

    env->ReleaseStringUTFChars(dataSource_, dataSource);
}

/**
 *  播放音视频.
 */
extern "C"
JNICALL void JNICALL
Java_com_hxj_ffmpegplayer_FFmpegPlayer_startNative(JNIEnv *env, jobject instance) {
    if (fFmpegPlayer) {
        // 异步线程播放.
        fFmpegPlayer->startAsync();
    }
}

/**
 * 渲染到 surfaceView.
 * @param src_data
 * @param width
 * @param height
 * @param src_lineSize
 */
void renderFrame(uint8_t *src_data, int width, int height, int src_lineSize) {
    pthread_mutex_lock(&mutex);

    if (!window) {
        pthread_mutex_unlock(&mutex);
    }

    ANativeWindow_setBuffersGeometry(
            window,
            width,
            height,
            WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        ANativeWindow_release(window);
        window = NULL;
        pthread_mutex_unlock(&mutex);
        return;
    }

    uint8_t *dst_data = static_cast<uint8_t *>(window_buffer.bits);
    int dst_lineSize = window_buffer.stride * 4;
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst_data + i * dst_lineSize, src_data + i * src_lineSize, dst_lineSize);
    }

    ANativeWindow_unlockAndPost(window);

    pthread_mutex_unlock(&mutex);
}

/**
 *  设置 surface.
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_hxj_ffmpegplayer_FFmpegPlayer_setSurfaceNative(JNIEnv *env, jobject instance,
                                                        jobject surface) {
    pthread_mutex_lock(&mutex);

    if (window) {
        ANativeWindow_release(window);
        window = NULL;
    }

    window = ANativeWindow_fromSurface(env, surface);
    pthread_mutex_unlock(&mutex);

}

extern "C"
JNICALL void JNICALL
Java_com_hxj_ffmpegplayer_FFmpegPlayer_stopNative(JNIEnv *env, jobject instance) {
    if (fFmpegPlayer) {
        fFmpegPlayer->stop();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_hxj_ffmpegplayer_FFmpegPlayer_releaseNative(JNIEnv *env, jobject instance) {
    pthread_mutex_lock(&mutex);

    if (window) {
        ANativeWindow_release(window);
        window = NULL;
    }
    pthread_mutex_unlock(&mutex);

    DELETE(fFmpegPlayer);
}
extern "C"
JNIEXPORT jint JNICALL
Java_com_hxj_ffmpegplayer_FFmpegPlayer_getDurationNative(JNIEnv *env, jobject instance) {
    if (fFmpegPlayer) {
        return fFmpegPlayer->getDuration();
    }

    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_hxj_ffmpegplayer_FFmpegPlayer_seekNative(JNIEnv *env, jobject instance,
                                                  jint playProgress) {
    if (fFmpegPlayer)
    {
        fFmpegPlayer->seek(playProgress);
    }

}