#include "AVGlobal.h"
#include "AVCallBackFunc.h"

void AVGlobal::SetAudioVideoSyncType(int eSyncTYpe)
{
    m_eTypeSync = eSyncTYpe;
}

int AVGlobal::GetStreamComponent(int nStreamIndex) {
    int nRet = -1;
    if(nStreamIndex < 0 || nStreamIndex > m_pAVformatContext->nb_streams)
    {
        return -1;
    }

    AVStream *pStream = m_pAVformatContext->streams[nStreamIndex];
    AVCodecID eCodexId = pStream->codecpar->codec_id;    //codex_id为编码类型id: AV_CODEC_ID_H264:24, AV_CODEC_ID_AAC,

    //根据codexId得到对应编码类型的编解码器
    //static const AVCodec * const codec_list[] = { 内部已经初始化过，所以直接就能拿到对应解码器，不需要new或者malloc
    const AVCodec *pCodec = avcodec_find_decoder(eCodexId);

    //分配解码器上下文
    AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
    if(NULL == pCodecContext)
    {
        av_log(NULL, AV_LOG_ERROR, "Failed to alloc AVCodecContext\n");
        goto __ERROR;
    }

    //拷贝AVStream的成员AVCodecParameters *codecpar 到解码器上下文内容
    nRet = avcodec_parameters_to_context(pCodecContext, pStream->codecpar);
    if(nRet < 0)
    {
        av_log(pCodecContext, AV_LOG_ERROR, "Couldn't copy codec parameters to codec context!\n");
        goto __ERROR;
    }

    //用于打开解码器，并将AVCodecContext和解码器绑定
    nRet = avcodec_open2(pCodecContext, pCodec, NULL);
    if(nRet < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Failed to bind codecCtx and codec!\n");
        goto __ERROR;
    }

    switch (pCodecContext->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
            //打开音频设备
            nRet = OpenAudioDevice(pCodecContext->ch_layout.nb_channels,pCodecContext->sample_rate);
            if(nRet < 0)
            {
                goto __ERROR;
            }

            m_pSreamAudio = pStream;
            m_pCodecCtxAudio = pCodecContext;

            //播放音频, 0为立即开启扬声器， 非0为过这么长时间再开启扬声器
            SDL_PauseAudio(0);

            break;
        case AVMEDIA_TYPE_VIDEO:
            m_pStreamVideo = pStream;
            m_pCodecCtxVideo = pCodecContext;

            //音视频同步的相关字段
            m_vframe_time = (double)av_gettime() / 1000000.0;  //换成的秒时间戳 1685954910   2023-06-05 16:48:30
            m_vframe_last_delay = 40e-3;  // 0.04
            m_video_current_pts_time = av_gettime();   //微秒时间戳：1685954910409425   2023-06-05 16:48:30

            m_pVideoDecodeThread = SDL_CreateThread(VideoDecodeThread, "video_decode_thread", this);

            break;
        default:
            av_log(pCodecContext, AV_LOG_ERROR, "Unknow Codec Type: %d\n", pCodecContext->codec_type);
            break;
    }

    __ERROR:
    return nRet;
}

int AVGlobal::OpenAudioDevice(int nChannles, int nSampleRate)
{
    //设置音频参数
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = nSampleRate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = nChannles;
    wanted_spec.silence = 0;    //静音的值
    wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;   //音频缓存区的大小
    wanted_spec.callback = CallBackSdlAudio;
    wanted_spec.userdata = (void *)this;

    SDL_AudioSpec obtain_spec;  //输出参数

    av_log(NULL, AV_LOG_INFO, "wanted spec:channels:%d, samp_fmt:%d, sample_rate:%d\n"
            ,wanted_spec.channels, wanted_spec.format, wanted_spec.freq);

    //wanted_spec：期望的参数。 obtain_spec：实际音频设备的参数，一般情况下设置为NULL即可。
    //里面会自动启动一个线程
    if(SDL_OpenAudio(&wanted_spec, &obtain_spec) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "SDL_OpenAudio: %s\n", SDL_GetError());
        return -1;
    }

    return obtain_spec.size;
}

int AVGlobal::DecodeAudioPacket()
{
    int nRet = 0;
    int nResamplesConvertPeerChannel = 0;
    int nOutputPcmByteSize = 0;

    for (;;) {
        if(m_queAudioPacket.GetPacketQueueElement(&m_audioPkt, false) <= 0)
        {
            av_log(NULL, AV_LOG_ERROR, "Can't get packet from audio queue");
            break;
        }

        //将编码的数据包发送给编解码器进行解码
        nRet = avcodec_send_packet(m_pCodecCtxAudio, &m_audioPkt);
        av_packet_unref(&m_audioPkt);
        if(nRet < 0)
        {
            av_log(m_pCodecCtxAudio, AV_LOG_ERROR, "Failed to send pkt to decoder!\n");
            goto __END;
        }

        while(nRet >= 0)
        {
            nRet = avcodec_receive_frame(m_pCodecCtxAudio, &m_audioFrame);
            if(nRet == AVERROR(EAGAIN) || nRet == AVERROR_EOF)
            {
                break;
            }
            else if(nRet < 0)
            {
                av_log(m_pCodecCtxAudio, AV_LOG_ERROR, "Failed to receive frame from decoder!\n");
                goto __END;
            }
        }

        //拿到解码后音频帧需要判断音频帧的参数与扬声器的是否一致的，如果不一致需要进行重采样
        if(m_pAudioSwrCtx == NULL)
        {
            AVChannelLayout ch_layout_in, ch_layout_out;
            av_channel_layout_copy(&ch_layout_in, &m_pCodecCtxAudio->ch_layout);
            av_channel_layout_copy(&ch_layout_out, &ch_layout_in);

            //重采样一般比较多的是针对采样格式的重采样
            if(m_pCodecCtxAudio->sample_fmt != AV_SAMPLE_FMT_S16)
            {
                swr_alloc_set_opts2(&m_pAudioSwrCtx, &ch_layout_out,
                                    AV_SAMPLE_FMT_S16, m_pCodecCtxAudio->sample_rate,
                                    &ch_layout_in,
                                    m_pCodecCtxAudio->sample_fmt, m_pCodecCtxAudio->sample_rate,
                                    0, NULL);

                swr_init(m_pAudioSwrCtx);
            }
        }

        if(m_pAudioSwrCtx)
        {
            const uint8_t  **buffAudioIn = (const uint8_t **)m_audioFrame.extended_data;
            uint8_t **bufferOut = &m_pcmbuffer;
            int nSampleCountOut = m_audioFrame.nb_samples + 256;    //采样点个数,加上了冗余部分

            //计算该帧pcm所占的字节数
            //dataSize = 2(通道数) * 2 (16bit,2B) * m_audioFrame.nb_samples(采样点个数);
            int nOutSize = av_samples_get_buffer_size(NULL, m_audioFrame.ch_layout.nb_channels,
                                                      nSampleCountOut, AV_SAMPLE_FMT_S16, 0);
            av_fast_malloc(m_pcmbuffer, &m_audio_buff_size, nOutSize);

            //返回重采样后的每个通道的采样点个数
            nResamplesConvertPeerChannel = swr_convert(m_pAudioSwrCtx, bufferOut, nSampleCountOut,
                        buffAudioIn, m_audioFrame.nb_samples);

            //根据采样点计算出重采样后的pcm数据所占用的字节
            nOutputPcmByteSize = nResamplesConvertPeerChannel * m_audioFrame.ch_layout.nb_channels *
                                 av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
        }

        //计算整个音频当前的播放时钟，音视频同步时使用
        if(!isnan(m_audioFrame.pts))
        {
            m_clock_audio = m_audioFrame.pts + (double)m_audioFrame.nb_samples / m_audioFrame.sample_rate;
        }
        else
        {
            m_clock_audio = NAN;
        }

        av_frame_unref(&m_audioFrame);

        return nOutputPcmByteSize;
    }

    __END:
    return nRet;
}