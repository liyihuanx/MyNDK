//
// Created by admin on 2022/4/7.
//

#ifndef MYNDK_JNICALLBACKHELPER_H
#define MYNDK_JNICALLBACKHELPER_H


#include <jni.h>
#include "NativeConstant.h"

class JNICallbackHelper {

private:
    JavaVM *vm = 0;
    JNIEnv *env = 0;
    jobject job;
    jmethodID jmd_prepared;
    jmethodID jmd_error;
    jmethodID jmd_progress;
    jmethodID jmd_status;
public:
    JNICallbackHelper(JavaVM *vm, JNIEnv *env, jobject job);

    virtual ~JNICallbackHelper();

    void prepareCallBack(int thread_mode);
    void errorCallBack(int thread_mode, int error_code);
    void progressCallBack(int thread_mode, int progress);
    void statusCallBack(int thread_mode, int msg);

};


#endif //MYNDK_JNICALLBACKHELPER_H
