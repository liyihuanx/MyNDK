#include <jni.h>
#include <string>
#include "NativePlayer.h"

extern "C"{
#include <libavutil/avutil.h>
}
using namespace std;




NativePlayer *nativePlayer = nullptr;
JavaVM *vm = nullptr;
jint JNI_OnLoad(JavaVM * vm, void * args) {
    ::vm = vm;
    return JNI_VERSION_1_6;
}

/**
 * 播放前初始化，准备
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_liyihuanx_myndk_SimplePlayer_nativePrepare(JNIEnv *env, jobject thiz,
                                                    jstring data_source) {

    auto *jniCallbackHelper = new JNICallbackHelper(vm, env, thiz);

    const char *dataSource = env->GetStringUTFChars(data_source, nullptr);
    // 创建播放器
    nativePlayer = new NativePlayer(dataSource, jniCallbackHelper);
    // 播放器准备工作
    nativePlayer->prepare();
    // 释放掉
    env->ReleaseStringUTFChars(data_source, dataSource);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_liyihuanx_myndk_SimplePlayer_nativeStart(JNIEnv *env, jobject thiz) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_liyihuanx_myndk_SimplePlayer_nativeStop(JNIEnv *env, jobject thiz) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_liyihuanx_myndk_SimplePlayer_nativePause(JNIEnv *env, jobject thiz) {

}
extern "C"
JNIEXPORT void JNICALL
Java_com_liyihuanx_myndk_SimplePlayer_nativeResume(JNIEnv *env, jobject thiz) {

}