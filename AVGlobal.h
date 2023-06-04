#ifndef AUDIOVIDEOPLAYER_AVGLOBAL_H
#define AUDIOVIDEOPLAYER_AVGLOBAL_H

#include "AVPacketQueue.h"
#include "VideoFrameQueue.h"

extern "C" {
    #include "libavformat/avformat.h"
    #include "libavutil/log.h"
    #include "SDL2/SDL_thread.h"
    #include "SDL2/SDL.h"
};

enum {
    AV_SYNC_AUDIO_MASTER,           //以音频为主同步
    AV_SYNC_VIDEO_MASTER,           //以视频为主同步
    AV_SYNC_EXTERNAL_CLOCK_MASTER   //以外部为主时钟同步
};

#define SDL_AUDIO_BUFFER_SIZE 1024

static void CallBackSdlAudio(void *userdata, Uint8 *stream, int len);

class AVGlobal {
public:
    void SetAudioVideoSyncType(int eSyncTYpe);
    int GetStreamComponent(int nStreamIndex);   //根据流索引去获取具体的音频和视频编码的组件
    int OpenAudioDevice(int nChannles, int nSampleRate);

public:
    //媒体文件相关
    char *m_pFileName = nullptr;
    AVFormatContext *m_pAVformatContext = NULL;

    //音视频同步相关
    int m_eTypeSync = 0;    //音视频同步的方式

    //音频相关
    AVPacketQueue m_queAudio;
    int m_nAudioIndex = -1;

    uint8_t *pcmbuffer;               //解码后的pcm数据存放的
    unsigned int audio_buff_size;     //pcmbuffer的大小
    unsigned int audio_buff_use_pos;  //buffer中已经使用的数据的位置

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
