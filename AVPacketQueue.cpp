#include "AVPacketQueue.h"

int AVPacketQueue::InitPacketQueue()
{
    m_pPacketList = av_fifo_alloc2(1, sizeof(MyAVPacketList), AV_FIFO_FLAG_AUTO_GROW);
    if(NULL == m_pPacketList)
    {
        return -1;
    }

    m_pMutex = SDL_CreateMutex();
    if(NULL == m_pMutex)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex():%s\n", SDL_GetError());
        return -2;
    }

    m_pCond = SDL_CreateCond();
    if(NULL == m_pCond)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond():%s\n", SDL_GetError());
        return -3;
    }
    return 0;
}

int AVPacketQueue::PushPacketQueue(AVPacket *pKt)
{
    AVPacket  *pKtTmp = av_packet_alloc();
    int nRet = -1;

    if(pKtTmp == NULL)
    {
        av_packet_unref(pKt);
        return  nRet;
    }

    av_packet_move_ref(pKtTmp, pKt);

    SDL_LockMutex(m_pMutex);
    nRet = PushPacketQueueInner(pKtTmp);
    SDL_UnlockMutex(m_pMutex);

    if(nRet < 0)
    {
        av_packet_free(&pKtTmp);
    }

    return nRet;
}

int AVPacketQueue::PushPacketQueueInner(AVPacket *pKt)
{
    MyAVPacketList pKtList;
    pKtList.pkt = pKt;
    int nRet = av_fifo_write(m_pPacketList, &pKtList, 1);
    if(nRet < 0)
    {
        return nRet;
    }

    m_nCountPackets++;
    m_nQueueSize += pKtList.pkt->size + sizeof(pKtList);
    m_duration += pKtList.pkt->duration;

    SDL_CondSignal(m_pCond);

    return 0;
}

int AVPacketQueue::GetPacketQueue(AVPacket *pKt, bool bBlock)
{

}