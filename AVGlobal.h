#ifndef AUDIOVIDEOPLAYER_AVGLOBAL_H
#define AUDIOVIDEOPLAYER_AVGLOBAL_H

#include "AVPacketQueue.h"
#include "VideoFrameQueue.h"

extern "C" {
    #include "libavformat/avformat.h"
    #include "SDL2/SDL_thread.h"
    #include "SDL2/SDL.h"
};

enum {
    AV_SYNC_AUDIO_MASTER,           //以音频为主同步
    AV_SYNC_VIDEO_MASTER,           //以视频为主同步
    AV_SYNC_EXTERNAL_CLOCK_MASTER   //以外部为主时钟同步
};

class AVGlobal {
public:
    void SetAudioVideoSyncType(int eSyncTYpe);
    int GetStreamComponent(int nStreamIndex);   //根据流索引去获取具体的音频和视频编码的组件

public:
    //媒体文件相关
    char *m_pFileName = nullptr;
    AVFormatContext *m_pAVformatContext = NULL;

    //音视频同步相关
    int m_eTypeSync = 0;    //音视频同步的方式

    //音频相关
    AVPacketQueue m_queAudio;
    int m_nAudioIndex = -1;

    //视频相关
    AVPacketQueue m_queVedio;
    VideoFrameQueue m_videoFrameQueue;    //解码后的视频帧队列
    int m_nVideoIndex = -1;
    SDL_Texture *m_pTextTure = NULL;

    //线程相关
    SDL_Thread   *m_pReadThread = NULL;
    SDL_Thread   *m_pDecodeThread = NULL;
    bool m_bQuit = false;
};

#endif //AUDIOVIDEOPLAYER_AVGLOBAL_H
