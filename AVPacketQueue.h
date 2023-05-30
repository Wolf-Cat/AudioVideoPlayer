#ifndef AUDIOVIDEOPLAYER_AVPACKETQUEUE_H
#define AUDIOVIDEOPLAYER_AVPACKETQUEUE_H

/*****
    用于存放解码前的音视频包
*****/

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
    int PushPacketQueue(AVPacket *pKt);
    int GetPacketQueueElement(AVPacket *pKtOut, bool bBlock);
    void PacketQueueClear();
    void PacketQueueDestory();

private:
    int PushPacketQueueInner(AVPacket *pKt);

private:

    AVFifo *m_pPacketList = NULL;
    int m_nCountPackets = 0;      //队列中有几个元素
    int m_nQueueSize = 0;
    int64_t m_duration = 0;       //队列中所有包加在一起可以播放的时长
    SDL_mutex *m_pMutex = NULL;
    SDL_cond *m_pCond = NULL;
};

#endif //AUDIOVIDEOPLAYER_AVPACKETQUEUE_H
