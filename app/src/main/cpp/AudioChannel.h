//
// Created by 黄小净 on 2019-11-05.
//

#ifndef FFMPEGPLAYER_AUDIOCHANNEL_H
#define FFMPEGPLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int stream_index, AVCodecContext *pContext);

    ~AudioChannel();
};


#endif //FFMPEGPLAYER_AUDIOCHANNEL_H
