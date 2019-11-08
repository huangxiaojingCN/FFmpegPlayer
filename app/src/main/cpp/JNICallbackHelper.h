//
// Created by 黄小净 on 2019-11-05.
//

#ifndef FFMPEGPLAYER_JNICALLBACKHELPER_H
#define FFMPEGPLAYER_JNICALLBACKHELPER_H

#include <jni.h>
#include "threadMode.h"


class JNICallbackHelper {
public:
    JNICallbackHelper(JavaVM *pVM, JNIEnv *pEnv, jobject pJobject);

    ~JNICallbackHelper();

    void onPrepared(int threadMode, int status);

private:
    JavaVM *javaVM;
    JNIEnv *env;
    jobject instance;
    jmethodID jmethod_prepared;
};


#endif //FFMPEGPLAYER_JNICALLBACKHELPER_H