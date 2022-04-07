//
// Created by admin on 2022/4/7.
//

#include "JNICallbackHelper.h"


JNICallbackHelper::JNICallbackHelper(JavaVM *vm, JNIEnv *env, jobject job) {
    this->vm = vm;
    this->env = env;

    // 坑： jobject不能跨越线程，不能跨越函数，必须全局引用
    // this->job = job;

    this->job = env->NewGlobalRef(job); // 提示全局引用

    jclass clazz = env->GetObjectClass(job);
    jmd_prepared = env->GetMethodID(clazz, "onPrepared", "()V");
}

JNICallbackHelper::~JNICallbackHelper() {
    env->DeleteGlobalRef(this->job);
    this->job = nullptr;
    this->vm = nullptr;
    this->env = nullptr;
}

void JNICallbackHelper::prepareCallBack(int thread_mode) {
    // JNI反射调用
    if (thread_mode == THREAD_MAIN) {
        // 主线程
        env->CallVoidMethod(job, jmd_prepared);
    } else if (thread_mode == THREAD_CHILD) {
        // 子线程 env也不可以跨线程
        JNIEnv *env_child;
        vm->AttachCurrentThread(&env_child, 0);
        env_child->CallVoidMethod(job, jmd_prepared);
        vm->DetachCurrentThread();
    }
}





