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

    return 0;
}
