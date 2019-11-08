#include <jni.h>
#include "log.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

#include "FFmpegPlayer.h"
#include "JNICallbackHelper.h"

JavaVM *javaVM = 0;
FFmpegPlayer *fFmpegPlayer;

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

extern "C"
JNICALL void JNICALL
Java_com_hxj_ffmpegplayer_FFmpegPlayer_stopNative(JNIEnv *env, jobject instance) {

}

extern "C"
JNIEXPORT void JNICALL
Java_com_hxj_ffmpegplayer_FFmpegPlayer_releaseNative(JNIEnv *env, jobject instance) {

}

