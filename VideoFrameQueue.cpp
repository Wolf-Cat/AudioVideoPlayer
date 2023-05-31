#include "VideoFrameQueue.h"

int VideoFrameQueue::InitVideoFrameQueue(int nMaxFrameCount)
{
    int nRet = 0;
    memset(m_queue, 0, sizeof(m_queue));
    m_pMutex = SDL_CreateMutex();
    if(NULL == m_pMutex)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex():%s\n", SDL_GetError());
        return -1;
    }

    m_pCond = SDL_CreateCond();
    if(NULL == m_pCond)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond():%s\n", SDL_GetError());
        return -2;
    }

    for(int i = 0; i < nMaxFrameCount; ++i)
    {
        m_queue[i].pVframe = av_frame_alloc();
        if(m_queue[i].pVframe == NULL)
        {
            return -3;
        }
    }

    return 0;
}