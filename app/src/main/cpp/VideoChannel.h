//
// Created by 黄小净 on 2019-11-05.
//

#ifndef FFMPEGPLAYER_VIDEOCHANNEL_H
#define FFMPEGPLAYER_VIDEOCHANNEL_H


#include "BaseChannel.h"
extern "C" {
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
};

class VideoChannel : public BaseChannel {

public:
    VideoChannel(int stream_index, AVCodecContext *pContext);

    ~VideoChannel();

    void start();

    pthread_t pid_video_decode;
    pthread_t pid_video_play;

    void video_decode();

    void video_play();
};


#endif //FFMPEGPLAYER_VIDEOCHANNEL_H
