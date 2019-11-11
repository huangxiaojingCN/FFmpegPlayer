//
// Created by 黄小净 on 2019-11-05.
//

#ifndef FFMPEGPLAYER_BASECHANNEL_H
#define FFMPEGPLAYER_BASECHANNEL_H

#include "log.h"
#include "safe_queue.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include <libavutil/time.h>
};

#include <pthread.h>


class BaseChannel {
public:
    BaseChannel(int stream_index, AVCodecContext *pContext, AVRational rational);

    virtual ~BaseChannel();

    bool isPlaying;

    AVCodecContext *avCodecContext;
    int stream_index;
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;

    static void releaseAVPacket(AVPacket **packet);

    static void releaseAVFrame(AVFrame **frame);

    AVRational time_base;
    double audio_time;
};


#endif //FFMPEGPLAYER_BASECHANNEL_H
