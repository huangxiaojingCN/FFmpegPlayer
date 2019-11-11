//
// Created by 黄小净 on 2019-11-05.
//

#ifndef FFMPEGPLAYER_AUDIOCHANNEL_H
#define FFMPEGPLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
extern "C" {
#include <libswresample/swresample.h>
#include <libavutil/time.h>
};

class AudioChannel : public BaseChannel{
public:
    AudioChannel(int stream_index, AVCodecContext *pContext, AVRational rational);

    ~AudioChannel();

    void start();

    void audio_decode();

    void audio_play();

private:
    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;
//引擎
    SLObjectItf engineObject;
//引擎接口
    SLEngineItf engineInterface;
//混音器
    SLObjectItf outputMixObject;
//播放器
    SLObjectItf bqPlayerObject;
//播放器接口
    SLPlayItf bqPlayerPlay;
//播放器队列接口
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

public:
    int out_channel;
    int out_sample_size;
    int out_sample_rate;
    int out_buffer_size;
    uint8_t *out_buffers;
    SwrContext *swr_ctx;

    int getPcm();
};


#endif //FFMPEGPLAYER_AUDIOCHANNEL_H
