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
