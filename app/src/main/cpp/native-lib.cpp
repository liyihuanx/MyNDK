#include "com_liyihuanx_myndk_SecondActivity.h"


using namespace std;


/**
 *
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_liyihuanx_myndk_SecondActivity_HelloJNI(JNIEnv *env, jobject thiz) {

    LOGD("Hello JNI");

}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_liyihuanx_myndk_SecondActivity_postString(JNIEnv *env, jobject thiz, jstring name) {

    // TODO 1.获取 --> 指针类型
    char *nameTemp = const_cast<char *>(env->GetStringUTFChars(name, nullptr));

    int nameLength = env->GetStringLength(name);

    for (int i = 0; i < nameLength; i++) {
        LOGD("nameTemp: %c", *(nameTemp + i));
        *(nameTemp + i) = 'c';
    }


    jstring result = env->NewStringUTF(nameTemp);

    // TODO 2.获取了 对应释放！
    env->ReleaseStringUTFChars(name, nameTemp);


    return result;
}