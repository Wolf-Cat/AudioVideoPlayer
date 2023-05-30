#ifndef AUDIOVIDEOPLAYER_VIDEOFRAMEQUEUE_H
#define AUDIOVIDEOPLAYER_VIDEOFRAMEQUEUE_H

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <SDL2/SDL_mutex.h>
};

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

/* 帧率的对象解释
 typedef struct AVRational{
    int num; ///< Numerator  分子
    int den; ///< Denominator  分母
} AVRational;
 */

class VideoFrameQueue {
public:
private:
    VideoFrame m_queue[9];
    int nRindex = 0;
    int nWindex = 0;
    int nSize = 0;
    int nAbort = 0;
    SDL_mutex *m_pMutex = NULL;
    SDL_cond *m_pCond = NULL;
};


#endif //AUDIOVIDEOPLAYER_VIDEOFRAMEQUEUE_H
