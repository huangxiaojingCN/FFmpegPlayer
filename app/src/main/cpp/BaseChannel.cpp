//
// Created by 黄小净 on 2019-11-05.
//

#include "BaseChannel.h"

BaseChannel::~BaseChannel() {
    packets.clear();
    frames.clear();
}

BaseChannel::BaseChannel(int stream_index, AVCodecContext *pContext) {
    this->avCodecContext = pContext;
    this->stream_index = stream_index;

    packets.setReleaseCallback(this->releaseAVPacket);
    frames.setReleaseCallback(this->releaseAVFrame);
}

void BaseChannel::releaseAVPacket(AVPacket **packet) {
    if (packet) {
        av_packet_free(packet);
        *packet = NULL;
    }
}

void BaseChannel::releaseAVFrame(AVFrame **frame) {
    if (frame) {
        av_frame_free(frame);
        *frame = NULL;
    }
}
