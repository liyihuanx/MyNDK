/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
#include <string>
#include <android/log.h>
/* Header for class com_liyihuanx_myndk_SecondActivity */


#define LOG_TAG "JNI_LOG"

//用于打印debug级别的log信息
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

//用于打印info级别的log信息
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)



#ifndef _Included_com_liyihuanx_myndk_SecondActivity
#define _Included_com_liyihuanx_myndk_SecondActivity
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_liyihuanx_myndk_SecondActivity
 * Method:    HelloJNI
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_liyihuanx_myndk_SecondActivity_HelloJNI
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif

extern "C"
JNIEXPORT jstring JNICALL
Java_com_liyihuanx_myndk_SecondActivity_postString(JNIEnv *env, jobject thiz, jstring name);




extern "C"
JNIEXPORT jobject JNICALL
Java_com_liyihuanx_myndk_SecondActivity_postDiffData(JNIEnv *env, jobject thiz, jintArray int_arr,
                                                     jobjectArray str, jobject array_list,
                                                     jobject user_bean);
extern "C"
JNIEXPORT jobject JNICALL
Java_com_liyihuanx_myndk_SecondActivity_createStu(JNIEnv *env, jobject thiz);


extern "C"
JNIEXPORT void JNICALL
Java_com_liyihuanx_myndk_SecondActivity_testQuote(JNIEnv *env, jobject thiz);