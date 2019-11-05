#include <jni.h>
#include "log.h"

extern "C" {
#include "libavcodec/avcodec.h"
}

#include "FFmpegPlayer.h"

extern "C"
JNIEXPORT jstring JNICALL
Java_com_hxj_ffmpegplayer_MainActivity_messageFromJNI(JNIEnv *env,
        jobject instance) {

    return env->NewStringUTF(av_version_info());
}

extern "C"
JNIEXPORT void JNICALL
 Java_com_hxj_ffmpegplayer_FFmpegPlayer_prepareNative(JNIEnv *env, jobject instance, jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, NULL);

    LOGD("dataSource: %s", dataSource);

    // 创建播放器. 将 url 传递给 FFmpegPlayer
    FFmpegPlayer *fFmpegPlayer = new FFmpegPlayer(dataSource);
    fFmpegPlayer->prepare();

    env->ReleaseStringUTFChars(dataSource_, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_hxj_ffmpegplayer_FFmpegPlayer_releaseNative(JNIEnv *env, jobject instance) {

}

extern "C"
JNICALL void JNICALL
Java_com_hxj_ffmpegplayer_FFmpegPlayer_startNative(JNIEnv *env, jobject instance) {

}

extern "C"
JNICALL void JNICALL
Java_com_hxj_ffmpegplayer_FFmpegPlayer_stopNative(JNIEnv *env, jobject instance) {

}
