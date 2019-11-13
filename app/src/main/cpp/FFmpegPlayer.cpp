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

    pthread_mutex_destroy(&seek_mutex);
}

FFmpegPlayer::FFmpegPlayer(const char *string, JNICallbackHelper *jniCallbackHelper) {
    // 由于 c/c++ 字符串后面以 \0 结束，所有要记得长度加1
    this->dataSource = new char[strlen(string) + 1];
    strcpy(this->dataSource, string);
    LOGD("FFmpegPlayer prepare dataSource: %s ", this->dataSource);

    this->jniCallbackHelper = jniCallbackHelper;

    pthread_mutex_init(&seek_mutex, NULL);
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
    pthread_create(&pid_prepare, NULL, task_prepare_async, this);
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

    // 获取视频时长 单位秒.
    duration = static_cast<int>(avFormatContext->duration / AV_TIME_BASE);

    // 循环遍历媒体流中音视频流或其他流的个数
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        // 获取流（音视频流）
        AVStream *stream = avFormatContext->streams[i];

        // 从流中获取解码这段流的参数
        AVCodecParameters *codecParameters = stream->codecpar;
        LOGD("codec_id: %d", codecParameters->codec_id);

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
            this->audioChannel = new AudioChannel(i, avCodecContext, stream->time_base);
            if (duration != 0) {
                audioChannel->setJniCallbackHelper(jniCallbackHelper);
            }
        } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            int fps = av_q2d(stream->avg_frame_rate);
            this->videoChannel = new VideoChannel(i, avCodecContext, stream->time_base, fps);
            videoChannel->setRenderCallback(this->callback);

            if (duration != 0) {
                videoChannel->setJniCallbackHelper(jniCallbackHelper);
            }
        }
    }

    // 未发现音视频
    if (!this->audioChannel && !this->videoChannel) {
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
        if (audioChannel && audioChannel->packets.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }

        if (videoChannel && videoChannel->packets.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }

        // 未解码的音视频包
        AVPacket *packet = av_packet_alloc();
        int ret = av_read_frame(avFormatContext, packet);
        if (!ret) {
            // 获取视频包.
            if (videoChannel && videoChannel->stream_index == packet->stream_index) {
                videoChannel->packets.push(packet);
            } else if (audioChannel && audioChannel->stream_index == packet->stream_index) {
                audioChannel->packets.push(packet);// 音频包.
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
        videoChannel->setAudioChannel(this->audioChannel);
        videoChannel->start();
    }

    if (audioChannel) {
        audioChannel->start();
    }

    isPlaying = 1;
    pthread_create(&pid_start, NULL, start_async, this);
}

void FFmpegPlayer::setRenderCallback(RenderCallback callback) {
    this->callback = callback;
}

void *task_stop(void *args) {
    FFmpegPlayer *fFmpegPlayer = static_cast<FFmpegPlayer *>(args);
    fFmpegPlayer->isPlaying = 0;

    // 等待解析和播放停止
    pthread_join(fFmpegPlayer->pid_start, NULL);
    pthread_join(fFmpegPlayer->pid_prepare, NULL);

    // 释放媒体流 formatContent
    if (fFmpegPlayer->avFormatContext) {
        avformat_close_input(&fFmpegPlayer->avFormatContext);
        avformat_free_context(fFmpegPlayer->avFormatContext);
        fFmpegPlayer->avFormatContext = NULL;
    }

    DELETE(fFmpegPlayer->audioChannel);
    DELETE(fFmpegPlayer->videoChannel);
    DELETE(fFmpegPlayer);

    return NULL;
}


/**
 *  停止播放.
 */
void FFmpegPlayer::stop() {
    pthread_create(&pid_stop, NULL, task_stop, this);
}

/**
 * 获取视频时长.
 * @return
 */
int FFmpegPlayer::getDuration() {
    return duration;
}

/**
 * 调节进度.
 * @param progress
 */
void FFmpegPlayer::seek(int progress) {
    this->progres = progress;
    if (this->progres < 0 || this->progres > duration)
    {
        return;
    }

    if (!audioChannel && !videoChannel)
    {
        return;
    }

    if (!avFormatContext)
    {
        return;
    }

    pthread_mutex_lock(&seek_mutex);
    int ret = av_seek_frame(avFormatContext, -1, this->progres * AV_TIME_BASE, AVSEEK_FLAG_BACKWARD);
    LOGD("设置进度: %d", ret);
    if (ret < 0)
    {
         if (jniCallbackHelper)
         {
             jniCallbackHelper->onPlayer(THREAD, -1);
         }

        LOGD("设置进度失败.");
        return;
    }

    if (audioChannel)
    {
        audioChannel->packets.setWork(0);
        audioChannel->frames.setWork(0);
        audioChannel->packets.clear();
        audioChannel->frames.clear();
        audioChannel->packets.setWork(1);
        audioChannel->frames.setWork(1);
    }

    if (videoChannel)
    {
        videoChannel->packets.setWork(0);
        videoChannel->frames.setWork(0);
        videoChannel->packets.clear();
        videoChannel->frames.clear();
        videoChannel->packets.setWork(1);
        videoChannel->frames.setWork(1);
    }

    pthread_mutex_unlock(&seek_mutex);
}
