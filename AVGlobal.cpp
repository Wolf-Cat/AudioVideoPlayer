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
    SDL_AudioSpec wanted_spec, obtain_spec;
    wanted_spec.freq = nSampleRate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = nChannles;
    wanted_spec.silence = 0;    //静音的值
    wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;   //音频缓存区的大小
    wanted_spec.callback = CallBackSdlAudio;
    wanted_spec.userdata = (void *)this;

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

/* stream 指向音频数据的buffer
 * len: 设备想要多少数据字节数
 * */
void CallBackSdlAudio(void *userdata, Uint8 *stream, int len)
{
    AVGlobal *pGlobal = (AVGlobal *)userdata;

    while(len > 0)
    {
        //判断解码后的PCM数据是否还有
    }
}