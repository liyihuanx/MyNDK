cmake_minimum_required(VERSION 3.10.2)

project("myndk")

# 批量导入
file(GLOB AllCPP *.c *.h *.cpp)

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
add_library(getndk SHARED IMPORTED)
set_target_properties(getndk PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../libs/${CMAKE_ANDROID_ARCH_ABI}/libgetndk.so)


target_link_libraries(
        myndk
        ${log-lib}
        getndk
)