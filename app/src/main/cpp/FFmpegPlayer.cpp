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

void *async_task_prepare(void *args) {
    FFmpegPlayer *fFmpegPlayer = (FFmpegPlayer *)args;
    fFmpegPlayer->prepareAsync();

    return 0;
}

void FFmpegPlayer::prepare() {
    pthread_t pt;
    pthread_create(&pt, NULL, async_task_prepare, this);
}

FFmpegPlayer::FFmpegPlayer(const char *string, JNICallbackHelper *jniCallbackHelper) {
    // 由于 c/c++ 字符串后面以 \0 结束，所有要记得长度加1
    this->dataSource = new char[strlen(string) + 1];
    strcpy(this->dataSource, string);
    LOGD("FFmpegPlayer prepare dataSource: %s ", this->dataSource);

    this->jniCallbackHelper = jniCallbackHelper;
}

/**
 *  异步解析处理，防止阻塞当前线程
 */
void FFmpegPlayer::prepareAsync() {

    AVFormatContext *avFormatContext = avformat_alloc_context();

    AVDictionary *dictionary = 0;

    int ret = av_dict_set(&dictionary, "timeout", "5000000", 0);
    if (ret < 0) {
        LOGE("设置超时失败.");
        if (jniCallbackHelper) {
            jniCallbackHelper->onPrepared(THREAD, -1);
        }
        return;
    }

    ret = avformat_open_input(&avFormatContext, this->dataSource, 0, &dictionary);
    // 释放字典
    av_dict_free(&dictionary);

    if (ret != 0) {
        LOGE("打开输入流失败");
        if (jniCallbackHelper) {
            jniCallbackHelper->onPrepared(THREAD, -1);
        }
        return;
    }

    ret = avformat_find_stream_info(avFormatContext, 0);
    if (ret < 0) {
        LOGE("查找流信息失败.");
        if (jniCallbackHelper) {
            jniCallbackHelper->onPrepared(THREAD, -1);
        }
        return;
    }

    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        // 获取流
        AVStream *stream = avFormatContext->streams[i];

        // 流的解码参数
        AVCodecParameters *codecParameters = stream->codecpar;

        AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
        if (!codec) {
            LOGE("未找到解码器");
            if (jniCallbackHelper) {
                jniCallbackHelper->onPrepared(THREAD, -1);
            }
            return;
        }

        // 获取解码器上下文
        AVCodecContext *avCodecContext = avcodec_alloc_context3(codec);
        if (!avCodecContext) {
            LOGE("解码器未找到");
            if (jniCallbackHelper) {
                jniCallbackHelper->onPrepared(THREAD, -1);
            }
            return;
        }

        ret = avcodec_parameters_to_context(avCodecContext, codecParameters);
        if (ret < 0) {
            LOGE("设置解码器上下文参数失败");
            if (jniCallbackHelper) {
                jniCallbackHelper->onPrepared(THREAD, -1);
            }
            return;
        }

        // 打开解码器.
        ret = avcodec_open2(avCodecContext,codec,0);
        if (ret != 0) {
            LOGE("打开解码器失败");
            if (jniCallbackHelper) {
                jniCallbackHelper->onPrepared(THREAD, -1);
            }
            return;
        }

        // 从解码器参数中获取流类型
        if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            this->audioChannel = new AudioChannel();
        } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            this->videoChannel = new VideoChannel();
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
