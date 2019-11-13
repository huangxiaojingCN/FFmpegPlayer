//
// Created by 黄小净 on 2019-11-05.
//

#include "AudioChannel.h"

AudioChannel::~AudioChannel() {
    if (swr_ctx)
    {
        swr_free(&swr_ctx);
        swr_ctx = NULL;
    }

    DELETE(out_buffers);
}

AudioChannel::AudioChannel(int stream_index, AVCodecContext *pContext, AVRational time_base)
        : BaseChannel(stream_index, pContext, time_base) {
    // 双声道
    out_channel = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    // 采样精度
    out_sample_size = av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
    out_sample_rate = 44100;
    out_buffer_size = out_channel * out_sample_rate * out_sample_size;
    out_buffers = static_cast<uint8_t *>(malloc(out_buffer_size));

    swr_ctx = swr_alloc_set_opts(0,
            AV_CH_LAYOUT_STEREO,
            AV_SAMPLE_FMT_S16,
            out_sample_rate,
            avCodecContext->channel_layout,
            avCodecContext->sample_fmt,
            avCodecContext->sample_rate,
            0, 0);

    swr_init(swr_ctx);
}

void *task_audio_decode(void *args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->audio_decode();

    return NULL;
}

void *task_audio_play(void *args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->audio_play();

    return NULL;
}

/**
 *  音频解码和播放
 */
void AudioChannel::start() {
    isPlaying = 1;
    packets.setWork(1);
    frames.setWork(1);
    pthread_create(&pid_audio_decode, NULL, task_audio_decode, this);
    pthread_create(&pid_audio_play, NULL, task_audio_play, this);
}

void AudioChannel::audio_decode() {
    AVPacket *packet = NULL;
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

        ret = avcodec_send_packet(avCodecContext, packet);
        if (ret) {
            break;
        }

        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret != 0) {
            releaseAVFrame(&frame);
            break;
        }

        frames.push(frame);
    }

    releaseAVPacket(&packet);
}

//4.3 创建回调函数
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    int pcm_size = audioChannel->getPcm();
    if (pcm_size > 0) {
        (*bq)->Enqueue(bq, audioChannel->out_buffers, pcm_size);
    }
}

void AudioChannel::audio_play() {
    SLresult result;
// 1.1 创建引擎对象：SLObjectItf engineObject
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
// 1.2 初始化引擎
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
// 1.3 获取引擎接口 SLEngineItf engineInterface
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    // 2.1 创建混音器：SLObjectItf outputMixObject
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0,
                                                 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
// 2.2 初始化混音器
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
//不启用混响可以不用获取混音器接口
// 获得混音器接口
//result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
//                                         &outputMixEnvironmentalReverb);
//if (SL_RESULT_SUCCESS == result) {
//设置混响 ： 默认。
//SL_I3DL2_ENVIRONMENT_PRESET_ROOM: 室内
//SL_I3DL2_ENVIRONMENT_PRESET_AUDITORIUM : 礼堂 等
//const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
//(*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
//       outputMixEnvironmentalReverb, &settings);
//}

//3.1 配置输入声音信息
//创建buffer缓冲类型的队列 2个队列
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                       2};
//pcm数据格式
//SL_DATAFORMAT_PCM：数据格式为pcm格式
//2：双声道
//SL_SAMPLINGRATE_44_1：采样率为44100
//SL_PCMSAMPLEFORMAT_FIXED_16：采样格式为16bit
//SL_PCMSAMPLEFORMAT_FIXED_16：数据大小为16bit
//SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT：左右声道（双声道）
//SL_BYTEORDER_LITTLEENDIAN：小端模式
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                                   SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_PCMSAMPLEFORMAT_FIXED_16,
                                   SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                                   SL_BYTEORDER_LITTLEENDIAN};

//数据源 将上述配置信息放到这个数据源中
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

//3.2 配置音轨（输出）
//设置混音器
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};
//需要的接口 操作队列的接口
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
//3.3 创建播放器
    result = (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject, &audioSrc, &audioSnk, 1, ids, req);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
//3.4 初始化播放器：SLObjectItf bqPlayerObject
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
//3.5 获取播放器接口：SLPlayItf bqPlayerPlay
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    //4.1 获取播放器队列接口：SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueue);

//4.2 设置回调 void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
    (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, this);

    (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);

    bqPlayerCallback(bqPlayerBufferQueue, this);

}

int AudioChannel::getPcm() {
    int pcm_data_size = 0;
    AVFrame *frame = NULL;
    while (isPlaying) {
        int ret = frames.pop(frame);
        if (!isPlaying) {
            break;
        }

        if (!ret) {
            continue;
        }

        int dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples,
                out_sample_rate, frame->sample_rate,
                AV_ROUND_UP);

        int samples_per_channel = swr_convert(swr_ctx, &out_buffers, dst_nb_samples,
                (const uint8_t **)(frame->data),
                    frame->nb_samples);

        pcm_data_size = samples_per_channel * out_sample_size * out_channel;

        // 读取每一帧的时间.
        audio_time = frame->best_effort_timestamp * av_q2d(time_base);
        if (jniCallbackHelper) {
            jniCallbackHelper->onProgress(THREAD, audio_time);
        }

        break;
    }

    releaseAVFrame(&frame);
    return pcm_data_size;
}

void AudioChannel::stop() {
    isPlaying = 0;
    packets.setWork(0);
    frames.setWork(0);
    pthread_join(pid_audio_decode, NULL);
    pthread_join(pid_audio_play, NULL);

//7.1 设置停止状态
    if (bqPlayerPlay) {
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
        bqPlayerPlay = 0;
    }
//7.2 销毁播放器
    if (bqPlayerObject) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = 0;
        bqPlayerBufferQueue = 0;
    }
//7.3 销毁混音器
    if (outputMixObject) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = 0;
    }
//7.4 销毁引擎
    if (engineObject) {
        (*engineObject)->Destroy(engineObject);
        engineObject = 0;
        engineInterface = 0;
    }
}
