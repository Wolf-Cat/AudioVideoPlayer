#ifndef AUDIOVIDEOPLAYER_AVGLOBAL_H
#define AUDIOVIDEOPLAYER_AVGLOBAL_H

#include "AVPacketQueue.h"

class AVGlobal {
public:
    AVPacketQueue m_queAudio;
    AVPacketQueue m_queVedio;
private:
    char *m_pFileName = nullptr;
    int m_nTypeSync = 0;    //音视频同步的方式

    int m_nAudioIndex = -1;
    int m_nVideoIndex = -1;
    bool m_bQuit = false;
};

#endif //AUDIOVIDEOPLAYER_AVGLOBAL_H
