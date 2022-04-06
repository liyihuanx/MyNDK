cmake_minimum_required(VERSION 3.10.2)

project("myndk")

# 批量导入
file(GLOB AllCPP ${CMAKE_SOURCE_DIR}/cpp/*.c
        ${CMAKE_SOURCE_DIR}/cpp/*.h
        ${CMAKE_SOURCE_DIR}/cpp/*.cpp)

add_library(
        myndk
        SHARED
        ${AllCPP})


find_library(
        log-lib
        log)
# 静态库导入
#add_library(getndk STATIC IMPORTED)
#set_target_properties(getndk PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/libgetndk.a)

# 动态库导入
#add_library(getndk SHARED IMPORTED)
#set_target_properties(getndk PROPERTIES
#        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/jniLibaa/${CMAKE_ANDROID_ARCH_ABI}/libgetndk.so)

# 第一步：导入fmod头文件
#include_directories("${CMAKE_SOURCE_DIR}/cpp/inc")
# 第二步：导入库文件 （方式一）
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/../../libs/${CMAKE_ANDROID_ARCH_ABI}")

message(WARNING "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}") # C:/Users/admin/Desktop/MyFFmpeg/app/src/main
message(WARNING "PROJECT_SOURCE_DIR = ${PROJECT_SOURCE_DIR}") # C:/Users/admin/Desktop/MyFFmpeg/app/src/main
message(WARNING "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}") # C:/Users/admin/Desktop/MyFFmpeg/app/src/main
message(WARNING "path = ${CMAKE_SOURCE_DIR}/jniLibaa/${CMAKE_ANDROID_ARCH_ABI}")


target_link_libraries(
        myndk
        ${log-lib}
        getndk
)