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