//
// Created by admin on 2022/4/8.
//

#ifndef MYNDK_SAFE_QUEUE_H
#define MYNDK_SAFE_QUEUE_H

/**
 * 线程安全的阻塞队列
 */

#include <queue>
#include <pthread.h>
#include "log4c.h"

using namespace std;

template<typename T> // 泛型：存放任意类型
class SafeQueue {
private:
    typedef void (*ReleaseCallback)(T *); // 函数指针 做回调 用来释放T里面的内容的
    typedef void (*SyncCallback)(queue<T> &);

    queue<T> queue;
    pthread_mutex_t mutex; // 互斥锁 安全
    pthread_cond_t cond; // 等待 和 唤醒
    int work;// 标记队列是否工作
    ReleaseCallback releaseCallback;
    SyncCallback syncCallback;
    bool isPause = false; // 是否暂停
public:
    SafeQueue() {
        // 初始化互斥锁
        pthread_mutex_init(&mutex, 0);
        // 初始化条件变量
        pthread_cond_init(&cond, 0);
    }

    ~SafeQueue() {
        // 释放
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void insertToQueue(T value) {
        // 1.上锁
        pthread_mutex_lock(&mutex); // 多线程的访问（先锁住）
        if (work) {
            // 2. 存入
            queue.push(value);
            // 3. 唤醒（当插入数据包 进队列后，要发出通知唤醒）
            if (!isPause) {
                pthread_cond_signal(&cond);
            }
        } else {
            // 5.不在工作状态 --> 不播放音视频了，清理队列和数据
            if (releaseCallback) {
                // 让外界释放我们的 value,
                // 为啥不能自己释放? 因为不明确value类型
                releaseCallback(&value);
            }
        }
        // 4.解锁
        pthread_mutex_unlock(&mutex);
    }

    int getQueueAndDel(T &value) {
        int result = 0;
        pthread_mutex_lock(&mutex);

        // 工作中，但是队列为空，就等待 || 暂停了进入等待
        while ((work && queue.empty()) || isPause) {
            pthread_cond_wait(&cond, &mutex);
        }
        if (!queue.empty()) { // 如果队列里面有数据，就进入此if
            // 取出队列的数据包 给外界，并删除队列数据包
            value = queue.front();
            queue.pop(); // 删除队列中的数据
            result = 1; // 成功了 Success 放回值  true
        }
        pthread_mutex_unlock(&mutex);
        return result;
    }

    void setWork(int work) {
        pthread_mutex_lock(&mutex); // 多线程的访问（先锁住）

        this->work = work;

        // 每次设置状态后，就去唤醒下，有没有阻塞睡觉的地方
        pthread_cond_signal(&cond);

        pthread_mutex_unlock(&mutex); // 多线程的访问（要解锁）
    }

    int empty() {
        return queue.empty();
    }

    int size() {
        return queue.size();
    }

    void clear() {
        pthread_mutex_lock(&mutex); // 多线程的访问（先锁住）

        unsigned int size = queue.size();

        for (int i = 0; i < size; ++i) {
            //循环释放队列中的数据
            T value = queue.front();
            if (releaseCallback) {
                releaseCallback(&value); // 让外界去释放堆区空间
            }
            queue.pop(); // 删除队列中的数据，让队列为0
        }

        pthread_mutex_unlock(&mutex); // 多线程的访问（要解锁）
    }

    /**
     * 设置此函数指针的回调，让外界去释放
     * @param releaseCallback
     */
    void setReleaseCallback(ReleaseCallback releaseCallback) {
        this->releaseCallback = releaseCallback;
    }


    void setSyncCallback(SyncCallback syncCallback) {
        this->syncCallback = syncCallback;
    }

    /**
     * 同步操作 丢包
     */
    void sync() {
        pthread_mutex_lock(&mutex);
        syncCallback(queue); // 函数指针 具体丢包动作，让外界完成
        pthread_mutex_unlock(&mutex);
    }


    void pause() {
        pthread_mutex_lock(&mutex);
        isPause = true;
        pthread_mutex_unlock(&mutex);
    }

    void resume() {
        pthread_mutex_lock(&mutex);
        isPause = false;
        // 恢复播放，要唤醒getQueueAndDel()
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

};


#endif //MYNDK_SAFE_QUEUE_H
