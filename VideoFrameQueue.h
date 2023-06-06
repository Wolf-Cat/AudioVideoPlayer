#ifndef AUDIOVIDEOPLAYER_VIDEOFRAMEQUEUE_H
#define AUDIOVIDEOPLAYER_VIDEOFRAMEQUEUE_H

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <SDL2/SDL_mutex.h>
};

class AVGlobal;

#define VIDEO_PICTURE_QUEUE_SIZE 9

/* 帧率的对象解释
 typedef struct AVRational{
    int num; ///< Numerator  分子
    int den; ///< Denominator  分母
} AVRational;
 */

typedef struct _VideoFrame {
    AVFrame *pVframe;
    double pts;
    double duration;
    int64_t pos;
    int nWidth;
    int nHight;
    int format;
    AVRational frameRate;
} VideoFrame;

class VideoFrameQueue {
public:
    int InitVideoFrameQueue(int nMaxFrameCount);
    int InsertFrame(AVGlobal *pGlobal, AVFrame *pAvFrameSrc, double pts, double duration, int64_t pos);
    void FrameQueuePop();
    VideoFrame* PeekWritableIndex();   //找到数组中可以放VideoFrame的坑位
    VideoFrame* FrameQueuePeek();      //返回当前可以拿去渲染的帧
    void UpdateQueueCanWriteIndex();   // 更新可以插入队列的帧的位置
    void DestoryFrameQueue();

    int m_nCountEle = 0;
private:
    VideoFrame m_queue[VIDEO_PICTURE_QUEUE_SIZE];
    int m_nRindex = 0;
    int m_nWindex = 0;    //可以write的index的位置
    bool m_isbort = false;
    SDL_mutex *m_pMutex = NULL;
    SDL_cond *m_pCond = NULL;
};

#endif //AUDIOVIDEOPLAYER_VIDEOFRAMEQUEUE_H
