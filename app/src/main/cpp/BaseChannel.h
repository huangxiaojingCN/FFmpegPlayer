//
// Created by 黄小净 on 2019-11-05.
//

#ifndef FFMPEGPLAYER_BASECHANNEL_H
#define FFMPEGPLAYER_BASECHANNEL_H

#include "log.h"
#include "safe_queue.h"

extern "C" {
#include "libavcodec/avcodec.h"
};


class BaseChannel {
public:
    BaseChannel(int stream_index, AVCodecContext *pContext);

    virtual ~BaseChannel();

    void releaseAVPacket(AVPacket **packet);

    void releaseAVFrame(AVFrame **frame);

    bool isPlaying;

    AVCodecContext *avCodecContext;
    int stream_index;
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
};


#endif //FFMPEGPLAYER_BASECHANNEL_H
