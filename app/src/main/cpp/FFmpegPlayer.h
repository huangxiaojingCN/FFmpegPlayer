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

extern "C" {
#include "libavformat/avformat.h"
};

class FFmpegPlayer {
public:
    FFmpegPlayer(const char *string, JNICallbackHelper *pHelper);

    ~FFmpegPlayer();

    void prepare();

    void prepareAsync();

    void start();

    void startAsync();

    void setRenderCallback(RenderCallback callback);

private:
    bool isPlaying;
    char *dataSource;

    AudioChannel *audioChannel;

    VideoChannel *videoChannel;
    JNICallbackHelper *jniCallbackHelper;

    AVFormatContext *avFormatContext;
    pthread_t start_tid;
    pthread_t pid_player;

    RenderCallback callback;
};


#endif //FFMPEGPLAYER_FFMPEGPLAYER_H
