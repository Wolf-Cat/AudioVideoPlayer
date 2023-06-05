#ifndef AUDIOVIDEOPLAYER_AVGLOBAL_H
#define AUDIOVIDEOPLAYER_AVGLOBAL_H

#include "AVPacketQueue.h"
#include "VideoFrameQueue.h"

extern "C" {
    #include "libavformat/avformat.h"
    #include "libavutil/log.h"
    #include "libavutil/time.h"
    #include "libswresample/swresample.h"     //音频重采样头文件
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
    int OpenAudioDevice(int nChannles, int nSampleRate);
    int DecodeAudioPacket();

public:
    //媒体文件相关
    char *m_pFileName = NULL;
    AVFormatContext *m_pAVformatContext = NULL;

    //音视频同步相关
    int m_eTypeSync = 0;              //音视频同步的方式
    double m_clock_audio = 0;         //音频当前的播放时钟
    double m_vframe_time = 0;         //视频当前播放的时间
    double m_vframe_last_delay = 0;   //上一次渲染视频帧的delay时间

    int64_t m_video_current_pts_time = 0;  //当前视频帧的pts的系统时间

    //音频相关
    int m_nAudioIndex = -1;
    AVPacketQueue m_queAudioPacket;
    AVCodecContext *m_pCodecCtxAudio = NULL;
    AVStream  *m_pSreamAudio = NULL;
    AVPacket m_audioPkt;
    AVFrame m_audioFrame;
    SwrContext *m_pAudioSwrCtx = NULL;

    uint8_t *m_pcmbuffer = NULL;               //解码后的pcm数据存放的
    unsigned int m_audio_buff_size = 0;     //pcmbuffer的大小
    unsigned int m_audio_buff_use_pos = 0;  //buffer中已经使用的数据的位置

    //视频相关
    int m_nVideoIndex = -1;
    AVStream  *m_pStreamVideo = NULL;
    AVCodecContext *m_pCodecCtxVideo = NULL;
    AVPacketQueue m_queVideoPacket;
    VideoFrameQueue m_videoFrameQueue;    //解码后的视频帧队列
    AVPacket  m_videoPkt;

    SDL_Texture *m_pTextTure = NULL;

    //线程相关
    SDL_Thread   *m_pReadThread = NULL;
    SDL_Thread   *m_pVideoDecodeThread = NULL;
    bool m_bQuit = false;
};

#endif //AUDIOVIDEOPLAYER_AVGLOBAL_H
