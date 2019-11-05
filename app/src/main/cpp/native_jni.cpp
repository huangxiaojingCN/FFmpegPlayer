#include <jni.h>

extern "C" {
#include "libavcodec/avcodec.h"
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_hxj_ffmpegplayer_MainActivity_messageFromJNI(JNIEnv *env,
        jobject instance) {

    return env->NewStringUTF(av_version_info());
}

