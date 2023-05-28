#ifndef AUDIOVIDEOPLAYER_AVPACKETQUEUE_H
#define AUDIOVIDEOPLAYER_AVPACKETQUEUE_H

extern "C" {
#include <SDL2/SDL_mutex.h>
};

class AVPacketQueue {
public:
    void InitPacketQueue();

private:
    int m_nCountPackets = 0;
    int m_nQueueSize = 0;
    int64_t m_duration = 0;
    SDL_mutex *m_pMutex = NULL;
    SDL_cond *m_pCond = NULL;
};

#endif //AUDIOVIDEOPLAYER_AVPACKETQUEUE_H
