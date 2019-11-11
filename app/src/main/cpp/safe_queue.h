#ifndef FFMPEGPLAYER_SAFE_QUEUE_H
#define FFMPEGPLAYER_SAFE_QUEUE_H

#include <pthread.h>
#include <queue>

using namespace std;

template <typename T>
class SafeQueue {

    // 交给外部释放.
    typedef void (* ReleaseCallback)(T *);
    typedef void (* SyncCallback)(queue<T> &);

public:
    SafeQueue() {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
    }

    ~SafeQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    // 入队
    void push(T value) {
        pthread_mutex_lock(&mutex);
        if (work) {
            q.push(value);
            pthread_cond_signal(&cond);
        } else {
            if (callback) {
                callback(&value);
            }
        }

        pthread_mutex_unlock(&mutex);
    }

    // 出队
    int pop(T &value) {
        int ret = 0;
        pthread_mutex_lock(&mutex);
        while (work && q.empty()) {
            // 等待数据.
            pthread_cond_wait(&cond, &mutex);
        }

        if (!q.empty()) {
            value = q.front();
            q.pop();
            ret = 1;
        }

        pthread_mutex_unlock(&mutex);
        return ret;
    }

    void setWork(int work) {
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    void setReleaseCallback(ReleaseCallback callback) {
        this->callback = callback;
    }


    void setSyncCallback(SyncCallback syncCallback) {
        this->syncCallback = syncCallback;
    }

    int empty() {
        return q.empty();
    }

    int size() {
        return q.size();
    }

    void clear() {
        pthread_mutex_lock(&mutex);
        unsigned int size = q.size();
        for (int i = 0; i < size; ++i) {
            T value = q.front();
            if (callback) {
                callback(&value);
            }
            q.pop();
        }
        pthread_mutex_unlock(&mutex);
    }

    void sync() {
        pthread_mutex_lock(&mutex);
        SyncCallback(q);
        pthread_mutex_unlock(&mutex);
    }

private:
    queue<T> q;
    int work;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    ReleaseCallback callback;
    SyncCallback syncCallback;
};

#endif //FFMPEGPLAYER_SAFE_QUEUE_H
