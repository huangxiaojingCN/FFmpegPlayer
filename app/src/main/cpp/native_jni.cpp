#include <jni.h>

extern "C"
JNIEXPORT jstring JNICALL
Java_com_hxj_ffmpegplayer_MainActivity_messageFromJNI(JNIEnv *env,
        jobject instance) {
    return env->NewStringUTF("Hello Java. I com from JNI.");
}

