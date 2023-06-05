#include "AVGlobal.h"

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
            nRet = OpenAudioDevice(pCodecContext->channels,pCodecContext->sample_rate);
            if(nRet < 0)
            {
                goto __ERROR;
            }

            m_pCodecCtxAudio = pCodecContext;

            break;
        case AVMEDIA_TYPE_VIDEO:
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
            AVChannelLayout ch_layout_in, cha_layout_out;
            av_channel_layout_copy(&ch_layout_in, &m_pCodecCtxAudio->ch_layout);
            av_channel_layout_copy(&cha_layout_out, &ch_layout_in);

            //重采样一般比较多的是针对采样格式的重采样
            if(m_pCodecCtxAudio->sample_fmt != AV_SAMPLE_FMT_S16)
            {
                swr_alloc_set_opts2(&m_pAudioSwrCtx, &cha_layout_out,
                                    AV_SAMPLE_FMT_S16, m_pCodecCtxAudio->sample_rate,
                                    &ch_layout_in,
                                    m_pCodecCtxAudio->sample_fmt, m_pCodecCtxAudio->sample_rate,
                                    0, NULL);

                swr_init(m_pAudioSwrCtx);
            }
        }
    }

    __END:
    return nRet;
}

/* stream 指向音频数据的buffer
 * len: 设备想要多少数据字节数
 * */
void CallBackSdlAudio(void *userdata, Uint8 *stream, int lenDeviceNeed)
{
    AVGlobal *pGlobal = (AVGlobal *)userdata;
    if(NULL == pGlobal)
    {
        return;
    }

    int lenCurBuffer = 0;  //目前队列中还剩的数据
    int lenCodec = 0;      //当次解码的数据的长度

    while(lenDeviceNeed > 0)
    {
        //先判断解码后的PCM数据buffer是否还有数据
        if(pGlobal->m_audio_buff_use_pos >= pGlobal->m_audio_buff_size)
        {
            //已经没有数据，需要进行解码packet
            lenCodec = pGlobal->DecodeAudioPacket();
            if(lenCodec > 0)
            {
                pGlobal->m_audio_buff_size = lenCodec;
            }
            else  //没有数据或者出错，则补静音数据
            {
                pGlobal->m_audio_buff_size = SDL_AUDIO_BUFFER_SIZE;   //默认补1024B
                pGlobal->m_pcmbuffer = NULL;
            }

            pGlobal->m_audio_buff_use_pos = 0;
        }

        //获取当前解码队列中还剩多少数据
        lenCurBuffer = pGlobal->m_audio_buff_size - pGlobal->m_audio_buff_use_pos;
        if(lenCurBuffer > lenDeviceNeed)
        {
            lenCurBuffer = lenDeviceNeed;
        }

        if (pGlobal->m_pcmbuffer != NULL)
        {
            memcpy(stream, (uint8_t *)pGlobal->m_pcmbuffer + pGlobal->m_audio_buff_use_pos, lenCurBuffer);
        }
        else
        {
            memset(stream, 0, lenCurBuffer);  //补1024个B的静音数据给声卡
        }

        lenDeviceNeed -= lenCurBuffer;
        stream += lenCurBuffer;
        pGlobal->m_audio_buff_use_pos += lenCurBuffer;
    }
}