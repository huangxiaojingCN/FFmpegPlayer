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

extern "C" {
#include "libavformat/avformat.h"
};

class FFmpegPlayer {
public:
    FFmpegPlayer(const char *string);

    ~FFmpegPlayer();

    void prepare();

    void prepareAsync();

private:
    char *dataSource;

    AudioChannel *audioChannel;

    VideoChannel *videoChannel;
};


#endif //FFMPEGPLAYER_FFMPEGPLAYER_H
