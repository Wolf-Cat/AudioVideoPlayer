#ifndef AUDIOVIDEOPLAYER_AVPACKETQUEUE_H
#define AUDIOVIDEOPLAYER_AVPACKETQUEUE_H

extern "C" {
#include <libavutil/fifo.h>
#include <libavcodec/packet.h>
#include <libavutil/log.h>
#include <SDL2/SDL_mutex.h>
};

typedef struct MyAVPacketList {
    AVPacket *pkt;
} MyAVPacketList;

class AVPacketQueue {
public:
    int InitPacketQueue();

private:
    AVFifo *m_pPacketList = NULL;
    int m_nCountPackets = 0;
    int m_nQueueSize = 0;
    int64_t m_duration = 0;
    SDL_mutex *m_pMutex = NULL;
    SDL_cond *m_pCond = NULL;
};

#endif //AUDIOVIDEOPLAYER_AVPACKETQUEUE_H
