#ifndef AUDIOVIDEOPLAYER_AVGLOBAL_H
#define AUDIOVIDEOPLAYER_AVGLOBAL_H

#include "AVPacketQueue.h"

enum {
    AV_SYNC_AUDIO_MASTER,           //以音频为主同步
    AV_SYNC_VIDEO_MASTER,           //以视频为主同步
    AV_SYNC_EXTERNAL_CLOCK_MASTER   //以外部为主时钟同步
};

class AVGlobal {
public:
    AVPacketQueue m_queAudio;
    AVPacketQueue m_queVedio;

    void SetAudioVideoSyncType(int eSyncTYpe);
private:
    char *m_pFileName = nullptr;
    int m_eTypeSync = 0;    //音视频同步的方式

    int m_nAudioIndex = -1;
    int m_nVideoIndex = -1;
    bool m_bQuit = false;
};

#endif //AUDIOVIDEOPLAYER_AVGLOBAL_H
