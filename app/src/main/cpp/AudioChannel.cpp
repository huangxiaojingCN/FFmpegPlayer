//
// Created by 黄小净 on 2019-11-05.
//

#include "AudioChannel.h"

AudioChannel::~AudioChannel() {

}

AudioChannel::AudioChannel(int stream_index, AVCodecContext *pContext) : BaseChannel(stream_index, pContext){

}
