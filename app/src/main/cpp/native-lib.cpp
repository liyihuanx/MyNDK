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

    LOGD("nameTemp: %s", nameTemp);

    int nameLength = env->GetStringLength(name);

//    for (int i = 0; i < nameLength; i++) {
//        LOGD("nameTemp: %c", *(nameTemp + i));
//        *(nameTemp + i) = 'c';
//    }


    jstring result = env->NewStringUTF(nameTemp);

    // TODO 2.获取了 记得要 对应释放！
    env->ReleaseStringUTFChars(name, nameTemp);


    return result;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_liyihuanx_myndk_SecondActivity_postDiffData(JNIEnv *env, jobject thiz, jintArray int_arr,
                                                     jobjectArray str, jobject array_list,
                                                     jobject user_bean) {

    int *arrTemp = env->GetIntArrayElements(int_arr, nullptr);
    int arrLength = env->GetArrayLength(int_arr);
    for (int i = 0; i < arrLength; ++i) {
        LOGD("int数组的值: %d", *(arrTemp + i));
    }
    /** mode 传值的类型
     * 0 : 刷新Java数组，并释放Native层数组(通常用这个)
     * JNI_COMMIT : 只刷新，不释放
     * JNI_ABORT : 只释放Native层数组
     *
     */
    env->ReleaseIntArrayElements(int_arr, arrTemp, 0);


    // ================== jobjectArray =====================

    jsize length = env->GetArrayLength(str);
    for (int i = 0; i < length; ++i) {
        // 变成 jstring 和 String处理差不多
        jstring strTemp = static_cast<jstring>(env->GetObjectArrayElement(str, i));
        char *charStr = (char *) env->GetStringUTFChars(strTemp, nullptr);

        LOGD("String 数组: %s \n", charStr);
        env->ReleaseStringUTFChars(strTemp, charStr);
    }

    // ============= jobject =============

    // 寻找类
    jclass listClazz = env->GetObjectClass(array_list);
    jmethodID get = env->GetMethodID(listClazz,"get", "(I)Ljava/lang/Object;");
    jmethodID size = env->GetMethodID(listClazz,"size", "()I");


//    jmethodID foreach = env->GetMethodID(listClazz,"forEach", "(Ljava/util/function/Consumer;)V");

    int listSize = 0;
    listSize = env->CallIntMethod(array_list, size);
    LOGD("ListSize: %d \n", listSize);

    for (int i = 0; i < listSize; ++i) {
        jstring temp = static_cast<jstring>(env->CallObjectMethod(array_list, get, i));
        char *charStr = (char *) env->GetStringUTFChars(temp, nullptr);

        LOGD("List: %s \n", charStr);

        env->ReleaseStringUTFChars(temp, charStr);
    }


    // ============= bean类 =============

    jclass user = env->GetObjectClass(user_bean);
    jmethodID toString = env->GetMethodID(user,"toString", "()Ljava/lang/String;");
    // jstring 都不能直接LOG，要转成指针，然后获取也要对应释放
    jstring userTemp = static_cast<jstring>(env->CallObjectMethod(user_bean, toString));
    char *charUser = (char *) env->GetStringUTFChars(userTemp, nullptr);
    LOGD("userTemp: %s \n", charUser);
    env->ReleaseStringUTFChars(userTemp, charUser);


    jmethodID setName = env->GetMethodID(user, "setUsername", "(Ljava/lang/String;)V");
    jstring newName = env->NewStringUTF("改名字");
    env->CallVoidMethod(user_bean, setName, newName);

    jmethodID setId = env->GetMethodID(user, "setId", "(I)V");
    env->CallVoidMethod(user_bean, setId, 1998);

    return user_bean;

}


extern "C"
JNIEXPORT jobject JNICALL
Java_com_liyihuanx_myndk_SecondActivity_createStu(JNIEnv *env, jobject thiz) {
    // 1.包名+类型 找到class
    const char * userBeanPath = "com/liyihuanx/myndk/UserBean";
    jclass userClazz = env->FindClass(userBeanPath);
    // 2.实例化对象，不调用构造函数
    jobject userBean = env->AllocObject(userClazz);


    // 实例化对象，调用构造函数
    jmethodID construct = env->GetMethodID(userClazz,"<init>", "()V");
    jobject userBean2 = env->NewObject(userClazz, construct);

    env->DeleteLocalRef(userBean2);
}



// 还是局部引用
jclass userCla = NULL;
extern "C"
JNIEXPORT void JNICALL
Java_com_liyihuanx_myndk_SecondActivity_testQuote(JNIEnv *env, jobject thiz) {
    // 第二次进入，不等于NULL
    if (NULL == userCla) {
        const char * userBeanPath = "com/liyihuanx/myndk/UserBean";
        userCla = env->FindClass(userBeanPath);
        // 提升为全局引用  -- 要手动释放
        // env->NewGlobalRef(jclass);
    }
    // 第二次拿不到值会崩溃
    jmethodID construct = env->GetMethodID(userCla,"<init>", "()V");
    jobject userBean2 = env->NewObject(userCla, construct);

    // 第一次执行，释放后但是不会为NULL，成为悬空指针
    env->DeleteLocalRef(userBean2);
    // 这只是规范，但是不会解决崩溃
    userBean2 = NULL;
}


// ============ 动态注册 ============
JavaVM * javavm = nullptr;

//
jstring dynamicRegister(JNIEnv *env, jobject thiz, jstring tag){
    char *nameTemp = const_cast<char *>(env->GetStringUTFChars(tag, nullptr));
    LOGD("nameTemp: %s", nameTemp);
    jstring result = env->NewStringUTF(nameTemp);
    env->ReleaseStringUTFChars(tag, nameTemp);
    return result;
}


static const JNINativeMethod gMethods[] = {
        {"dynamicRegister", "(Ljava/lang/String;)Ljava/lang/String;", (jstring *) dynamicRegister}
};

const char* secondActivityClass = "com/liyihuanx/myndk/SecondActivity";

JNIEXPORT jint JNI_OnLoad(JavaVM *javavm, void *) {

    ::javavm = javavm;

    JNIEnv *jniEnv = nullptr;
    int result = javavm->GetEnv(reinterpret_cast<void **>(&jniEnv), JNI_VERSION_1_6);

    if (result != JNI_OK) {
        return -1;
    }
//    jclass clazz, const JNINativeMethod* methods,jint nMethods

//    typedef struct {
//        const char* name;        // java层方法名
//        const char* signature;   // 签名
//        void*       fnPtr;       // 函数指针
//    } JNINativeMethod;

    jclass clazz = jniEnv->FindClass(secondActivityClass);

    jniEnv->RegisterNatives(clazz, gMethods, sizeof(gMethods) / sizeof(JNINativeMethod));

    LOGD("JNI_ONLOAD");
    return JNI_VERSION_1_6;
}

extern "C"{
    extern int get();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_liyihuanx_myndk_SecondActivity_testNdk(JNIEnv *env, jobject thiz) {
    LOGD("交叉编译的库： %d" ,get());
}