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
    void SetAudioVideoSyncType(int eSyncTYpe);

public:
    //媒体文件相关
    char *m_pFileName = nullptr;

    //音视频同步相关
    int m_eTypeSync = 0;    //音视频同步的方式

    //音频相关
    AVPacketQueue m_queAudio;
    int m_nAudioIndex = -1;

    //视频相关
    AVPacketQueue m_queVedio;
    int m_nVideoIndex = -1;

    //线程相关
    bool m_bQuit = false;
};

#endif //AUDIOVIDEOPLAYER_AVGLOBAL_H
