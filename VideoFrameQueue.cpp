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

int VideoFrameQueue::InsertFrame(AVGlobal *pGlobal, AVFrame *pAvFrameSrc, double pts, double duration, int64_t pos)
{
    VideoFrame *pCustomVFrame = PeekWritableIndex();

    if (pCustomVFrame == NULL)
    {
        return -1;
    }

    pCustomVFrame->frameRate = pAvFrameSrc->sample_aspect_ratio;
    pCustomVFrame->nWidth = pAvFrameSrc->width;
    pCustomVFrame->nHight = pAvFrameSrc->height;
    pCustomVFrame->format = pAvFrameSrc->format;

    pCustomVFrame->pts = pts;
    pCustomVFrame->duration = duration;
    pCustomVFrame->pos = pos;

    av_frame_move_ref(pCustomVFrame->pVframe, pAvFrameSrc);
    UpdateQueueCanWriteIndex();
    return 0;
}

VideoFrame* VideoFrameQueue::PeekWritableIndex()
{
    //需要等到一个坑位可以放置新的Frame
    SDL_LockMutex(m_pMutex);
    while(m_nCountEle >= VIDEO_PICTURE_QUEUE_SIZE && m_isbort == false)
    {
        SDL_CondWait(m_pCond, m_pMutex);
    }
    SDL_UnlockMutex(m_pMutex);

    return &m_queue[m_nWindex];
}

void VideoFrameQueue::UpdateQueueCanWriteIndex()
{
    if(++m_nWindex == VIDEO_PICTURE_QUEUE_SIZE)
    {
        m_nWindex = 0;
    }

    SDL_LockMutex(m_pMutex);
    m_nCountEle++;
    SDL_CondSignal(m_pCond);
    SDL_UnlockMutex(m_pMutex);
}

void VideoFrameQueue::DestoryFrameQueue()
{
    for (int i = 0; i < VIDEO_PICTURE_QUEUE_SIZE; ++i)
    {
        VideoFrame *pVideoframe = &m_queue[i];
        av_frame_unref(pVideoframe->pVframe);
        av_frame_free(&pVideoframe->pVframe);
    }

    SDL_DestroyMutex(m_pMutex);
    SDL_DestroyCond(m_pCond);
}