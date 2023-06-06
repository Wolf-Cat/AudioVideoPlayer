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

int AVPacketQueue::GetPacketQueueElement(AVPacket *pKtOut, bool bBlock)
{
    MyAVPacketList pktListTmp;
    int nRet = -1;
    SDL_LockMutex(m_pMutex);

    for (;;)
    {
        if(av_fifo_read(m_pPacketList, &pktListTmp, 1) >= 0)
        {
            m_nCountPackets--;
            m_nQueueSize -= pktListTmp.pkt->size + sizeof(pktListTmp);
            m_duration -= pktListTmp.pkt->duration;
            av_packet_move_ref(pKtOut, pktListTmp.pkt);   //将所有权转移给传出参数
            av_packet_free(&pktListTmp.pkt);
            nRet = 1;
            break;
        }
        else
        {
            if(bBlock)
            {
                SDL_CondWait(m_pCond, m_pMutex);
            }
            else
            {
                nRet = 0;
                break;
            }
        }
    }

    SDL_UnlockMutex(m_pMutex);
    return nRet;
}

int AVPacketQueue::GetQueueSize()
{
    return m_nQueueSize;
}

void AVPacketQueue::PacketQueueClear()
{
    MyAVPacketList pktListTmp;

    SDL_LockMutex(m_pMutex);
    while(av_fifo_read(m_pPacketList, &pktListTmp, 1) >= 0)
    {
        av_packet_free(&pktListTmp.pkt);
    }

    m_nCountPackets = 0;
    m_nQueueSize = 0;
    m_duration = 0;
    SDL_UnlockMutex(m_pMutex);
}

void AVPacketQueue::PacketQueueDestory()
{
    PacketQueueClear();
    av_fifo_freep2(&m_pPacketList);

    SDL_DestroyMutex(m_pMutex);
    SDL_DestroyCond(m_pCond);
}