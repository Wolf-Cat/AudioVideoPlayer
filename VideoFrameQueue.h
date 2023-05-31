#ifndef AUDIOVIDEOPLAYER_VIDEOFRAMEQUEUE_H
#define AUDIOVIDEOPLAYER_VIDEOFRAMEQUEUE_H

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <SDL2/SDL_mutex.h>
};

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

private:
    VideoFrame m_queue[VIDEO_PICTURE_QUEUE_SIZE];
    int nRindex = 0;
    int nWindex = 0;
    int nSize = 0;
    int nAbort = 0;
    SDL_mutex *m_pMutex = NULL;
    SDL_cond *m_pCond = NULL;
};

#endif //AUDIOVIDEOPLAYER_VIDEOFRAMEQUEUE_H
