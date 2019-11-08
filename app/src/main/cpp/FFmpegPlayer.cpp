//
// Created by 黄小净 on 2019-11-05.
//

#include "FFmpegPlayer.h"
#include "JNICallbackHelper.h"

FFmpegPlayer::~FFmpegPlayer() {
    if (dataSource) {
        delete dataSource;
        dataSource = NULL;
    }
}

FFmpegPlayer::FFmpegPlayer(const char *string, JNICallbackHelper *jniCallbackHelper) {
    // 由于 c/c++ 字符串后面以 \0 结束，所有要记得长度加1
    this->dataSource = new char[strlen(string) + 1];
    strcpy(this->dataSource, string);
    LOGD("FFmpegPlayer prepare dataSource: %s ", this->dataSource);

    this->jniCallbackHelper = jniCallbackHelper;
}

void *start_async(void *args) {
    FFmpegPlayer *fFmpegPlayer = static_cast<FFmpegPlayer *>(args);
    fFmpegPlayer->start(); // 解析视频包，将其发送到队列中进一步解码和播放.

    return 0;
}

void *task_prepare_async(void *args) {
    FFmpegPlayer *fFmpegPlayer = (FFmpegPlayer *)args;
    fFmpegPlayer->prepare(); // 真正解析
    return NULL;
}

void FFmpegPlayer::prepareAsync() {
    pthread_create(&pid_player, NULL, task_prepare_async, this);
}

/**
 *  异步解析处理，防止阻塞当前线程
 */
void FFmpegPlayer::prepare() {

    // 流媒体封装格式
    avFormatContext = avformat_alloc_context();

    AVDictionary *dictionary = 0;

    // 设置超时时间.
    int ret = av_dict_set(&dictionary, "timeout", "5000000", 0);
    if (ret < 0) {
        if (jniCallbackHelper) {
            jniCallbackHelper->onPrepared(THREAD, -1);
        }
        return;
    }

    /**
     * 打开媒体流
     * 参数1： AVFromatContext
     * 参数2: 媒体播放地址.
     * 参数3: 输入封装格式
     * 参数4: 解封装媒体流参数
     */
    ret = avformat_open_input(&avFormatContext, this->dataSource, 0, &dictionary);
    // 释放字典
    av_dict_free(&dictionary);

    if (ret != 0) {
        if (jniCallbackHelper) {
            jniCallbackHelper->onPrepared(THREAD, -1);
        }
        return;
    }

    // 查找媒体流中音视频流信息
    ret = avformat_find_stream_info(avFormatContext, 0);
    if (ret < 0) {
        if (jniCallbackHelper) {
            jniCallbackHelper->onPrepared(THREAD, -1);
        }
        return;
    }

    // 循环遍历媒体流中音视频流或其他流的个数
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        // 获取流（音视频流）
        AVStream *stream = avFormatContext->streams[i];

        // 从流中获取解码这段流的参数
        AVCodecParameters *codecParameters = stream->codecpar;

        // 通过流的编解码参数中编解码ID，获取当前流的解码器
        AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
        if (!codec) {
            if (jniCallbackHelper) {
                jniCallbackHelper->onPrepared(THREAD, -1);
            }
            return;
        }

        // 创建解码器的上下文
        AVCodecContext *avCodecContext = avcodec_alloc_context3(codec);
        if (!avCodecContext) {
            if (jniCallbackHelper) {
                jniCallbackHelper->onPrepared(THREAD, -1);
            }
            return;
        }

        // 设置解码器上下文参数
        ret = avcodec_parameters_to_context(avCodecContext, codecParameters);
        if (ret < 0) {
            if (jniCallbackHelper) {
                jniCallbackHelper->onPrepared(THREAD, -1);
            }
            return;
        }

        // 打开解码器.
        ret = avcodec_open2(avCodecContext,codec,0);
        if (ret != 0) {
            if (jniCallbackHelper) {
                jniCallbackHelper->onPrepared(THREAD, -1);
            }
            return;
        }

        // 从解码器参数中获取流类型
        if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            this->audioChannel = new AudioChannel(i, avCodecContext);
        } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            this->videoChannel = new VideoChannel(i, avCodecContext);
        }
    }

    // 未发现音视频
    if (!this->audioChannel && !this->videoChannel) {
        LOGE("当前文件未发现音视频.");
        if (jniCallbackHelper) {
            jniCallbackHelper->onPrepared(THREAD, -1);
        }
        return;
    }

    // 回调 java 层
    if (jniCallbackHelper) {
        jniCallbackHelper->onPrepared(THREAD, 0);
    }

}

/**
 *  解析音视频包将其发送到队列中.
 */
void FFmpegPlayer::start() {
    while (isPlaying) {
        // 未解码的音视频包
        AVPacket *packet = av_packet_alloc();
        int ret = av_read_frame(avFormatContext, packet);
        if (!ret) {
            // 获取一段视频包
            if (videoChannel && videoChannel->stream_index == packet->stream_index) {
                videoChannel->packets.push(packet);
            } else if (audioChannel && audioChannel->stream_index == packet->stream_index) {

            }
        } else if (ret == AVERROR_EOF) {

        } else {
            break;
        }
    }

    isPlaying = 0;
}

/**
 *  异步播放
 */
void FFmpegPlayer::startAsync() {
    if (videoChannel) {
        videoChannel->start();
    }

    isPlaying = 1;
    pthread_create(&start_tid, NULL, start_async, this);
}
