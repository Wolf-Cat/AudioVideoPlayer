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

    //根据AVCodec结构体，初始化AVCodecContext
    nRet = avcodec_open2(pCodecContext, pCodec, NULL);
    if(nRet < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Failed to bind codecCtx and codec!\n");
        goto __ERROR;
    }

    switch (pCodecContext->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
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
