#include "VideoChannel.h"

void dropAVFrame(queue<AVFrame *> &q) {
    while (!q.empty()) {
        AVFrame *frame = q.front();
        BaseChannel::releaseAVFrame(&frame);
        q.pop();
    }
}

void dropAVPacket(queue<AVPacket *> &q) {
    while (!q.empty()) {
        AVPacket *packet = q.front();
        if (packet->flags != AV_PKT_FLAG_KEY) {
            BaseChannel::releaseAVPacket(&packet);
            q.pop();
        } else {
            break;
        }
    }
}

VideoChannel::VideoChannel(int stream_index, AVCodecContext *pContext, AVRational rational, int fps)
        : BaseChannel(
        stream_index, pContext, rational) {
    this->fps = fps;
    packets.setSyncCallback(dropAVPacket);
    frames.setSyncCallback(dropAVFrame);
}

VideoChannel::~VideoChannel() {

}

void *task_video_decode(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_decode();

    return NULL;
}

void *task_video_play(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->video_play();

    return NULL;
}

/**
 *  视频播放： 1. 解码 2. 播放
 */
void VideoChannel::start() {
    isPlaying = 1;
    packets.setWork(1);
    frames.setWork(1);
    pthread_create(&pid_video_decode, 0, task_video_decode, this);
    pthread_create(&pid_video_play, 0, task_video_play, this);
}

/**
 *  视频解码
 */
void VideoChannel::video_decode() {
    AVPacket *packet = 0;
    while (isPlaying) {
        if (isPlaying && frames.size() > 100) {
            av_usleep(10 * 1000);
            continue;
        }
        int ret = packets.pop(packet);
        if (!isPlaying) {
            break;
        }

        if (!ret) {
            continue;
        }

        // 取一个待解码的视频数据包
        ret = avcodec_send_packet(avCodecContext, packet);
        if (ret) {
            break;
        }

        // 解码后的视频原始数据包.
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            releaseAVFrame(&frame);
            continue;
        } else if (ret != 0) {
            releaseAVFrame(&frame);
            break;
        }

        // 取到解码后的视频数据包，加入到 frames 队列
        frames.push(frame);
    }

    // 释放 packet 包.
    releaseAVPacket(&packet);
}

/**
 *  播放 将 yuv 转成 rgba，交给 surfaceView 渲染.
 */
void VideoChannel::video_play() {
    uint8_t  *dst_data[4];
    int dst_linesize[4];
    AVFrame *frame = NULL;

    // yun -> rgba.
    SwsContext *swsContext = sws_getContext(
            avCodecContext->width,avCodecContext->height,
            avCodecContext->pix_fmt,
            avCodecContext->width,avCodecContext->height,
            AV_PIX_FMT_RGBA,
            SWS_BILINEAR,
            NULL,
            NULL,
            NULL
    );

    // 申请内存空间.
    av_image_alloc(dst_data,
            dst_linesize,
            avCodecContext->width,avCodecContext->height,
            AV_PIX_FMT_RGBA,
            1);

    while (isPlaying) {
        int ret = frames.pop(frame);
        if (!ret) {
            continue;
        }

        if (!isPlaying) {
            break;
        }

        sws_scale(swsContext,
                frame->data,frame->linesize,
                0,
                avCodecContext->height,
                dst_data,dst_linesize);

        double extra_delay =  frame->repeat_pict / (2 * fps);
        double base_delay = 1.0 / fps;
        double  real_delay = base_delay + extra_delay;

        double  video_time = frame->best_effort_timestamp * av_q2d(time_base);
        if (!audio_channel) {
            av_usleep(real_delay * 1000000);
            if (jniCallbackHelper)
            {
                jniCallbackHelper->onProgress(THREAD, video_time);
            }
        } else {
            double time_diff = video_time - audio_channel->audio_time;
            if (time_diff > 0) {
                if (time_diff > 1) {
                    av_usleep(real_delay * 2 * 1000000);
                } else {
                    av_usleep((real_delay + time_diff) * 1000000);
                }
            } else if (time_diff < 0){
                if (fabs(time_diff) >= 0.05) {
                    frames.sync();
                    continue;
                }
            }
        }

        // 回调渲染.
        if (renderCallback) {
            renderCallback(
                    dst_data[0],
                    avCodecContext->width, avCodecContext->height,
                    dst_linesize[0]);
        }
        // 释放 frame.
        releaseAVFrame(&frame);
    }

    releaseAVFrame(&frame);
    isPlaying = 0;
    av_freep(&dst_data[0]);
    sws_freeContext(swsContext);
}

void VideoChannel::setRenderCallback(RenderCallback callback) {
    this->renderCallback = callback;
}

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {
    this->audio_channel = audioChannel;
}
