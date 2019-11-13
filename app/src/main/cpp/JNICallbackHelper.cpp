#include "JNICallbackHelper.h"

JNICallbackHelper::~JNICallbackHelper() {
    javaVM = NULL;
    env->DeleteGlobalRef(instance);
    instance = NULL;
    env = NULL;

}

JNICallbackHelper::JNICallbackHelper(JavaVM *javaVM, JNIEnv *env, jobject instance) {
    this->javaVM = javaVM;
    this->env = env;
    this->instance = env->NewGlobalRef(instance);

    jclass clazz = env->GetObjectClass(this->instance);
    this->jmethod_prepared = env->GetMethodID(clazz, "onPrepared", "(I)V");
    this->jmethod_progress = env->GetMethodID(clazz, "onProgress", "(I)V");
    this->jmethod_player = env->GetMethodID(clazz, "onPlayer", "(I)V");
}

void JNICallbackHelper::onPrepared(int threadMode, int status) {
    if (threadMode == MAIN) {
        env->CallVoidMethod(instance, jmethod_prepared, status);
    } else {
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmethod_prepared, status);
        javaVM->DetachCurrentThread();
    }
}

void JNICallbackHelper::onProgress(int threadMode, int progress) {
    if (threadMode == MAIN) {
        env->CallVoidMethod(instance, jmethod_progress, progress);
    }
    else
    {
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(instance, jmethod_progress, progress);
        javaVM->DetachCurrentThread();
    }
}

void JNICallbackHelper::onPlayer(int threadMode, int status) {
    if (threadMode == MAIN)
    {
        env->CallVoidMethod(instance, jmethod_player, status);
    }
    else
    {
        JNIEnv *env_child;
        javaVM->AttachCurrentThread(&env_child, NULL);
        env->CallVoidMethod(instance, jmethod_player, status);
        javaVM->DetachCurrentThread();
    }
}

