//
// Created by 黄小净 on 2019-11-05.
//

#ifndef FFMPEGPLAYER_FFMPEGPLAYER_H
#define FFMPEGPLAYER_FFMPEGPLAYER_H

#include "log.h"
#include <cstring>
#include <pthread.h>
#include "AudioChannel.h"
#include "VideoChannel.h"
#include "JNICallbackHelper.h"
#include "macro.h"

extern "C" {
#include "libavformat/avformat.h"
};

class FFmpegPlayer {

    friend void *task_stop(void *args);

public:
    FFmpegPlayer(const char *string, JNICallbackHelper *pHelper);

    ~FFmpegPlayer();

    void prepare();

    void prepareAsync();

    void start();

    void startAsync();

    void setRenderCallback(RenderCallback callback);

    void stop();

    int getDuration();

    void seek(int progress);
private:
    bool isPlaying;
    char *dataSource;

    AudioChannel *audioChannel;

    VideoChannel *videoChannel;
    JNICallbackHelper *jniCallbackHelper;

    AVFormatContext *avFormatContext;

    RenderCallback callback;
    pthread_t pid_stop;
    pthread_t pid_prepare;
    pthread_t pid_start;
    int duration;
    pthread_mutex_t seek_mutex;
    int progres;
};


#endif //FFMPEGPLAYER_FFMPEGPLAYER_H
