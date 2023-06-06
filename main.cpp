#include <iostream>
extern "C" {
    #include <libavutil/log.h>
    #include <libavformat/avformat.h>
    #include <libavutil/error.h>
    #include <SDL2/SDL.h>
}

#include "AVGlobal.h"
#include "AVCallBackFunc.h"

using namespace std;

namespace {
    const int WIN_WIDTH = 640;
    const int WIN_HEIGHT = 480;
}

#define FF_REFRESH_EVENT  (SDL_USEREVENT)

static SDL_Window *g_pWindow = NULL;
static SDL_Renderer *g_pRenderer = NULL;

int PlayerInit(const char* pFileName, AVGlobal *pAVGlobal);
void SdlEventLoop(AVGlobal *pAVGlobal);
void ScheduleRefresh(AVGlobal *pGlobal, int timerLen);
Uint32 SdlRefreshTimerCallBack(Uint32 interval, void *arg);
void VideoFrameRefresh(void *arg);
void RenderPicture(AVGlobal *pGlobal);

int main(int argc, char *agrv[])
{
    av_log_set_level(AV_LOG_DEBUG);
    av_log(NULL, AV_LOG_INFO, "Hello ffmpeg\n");

    uint32_t version = avformat_version();
    cout << "version:" << version << endl;

    char input_file_path[] = "./testVedio.mp4";
    int nRet = -1;

    AVGlobal* pAVglobal = new AVGlobal;

    if(SDL_Init(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == 0))
    {
        std::cout << "SDL init success" << std::endl;
    }

    g_pWindow = SDL_CreateWindow("Cat Audio Video",
                                 SDL_WINDOWPOS_UNDEFINED,
                                 SDL_WINDOWPOS_UNDEFINED,
                                 WIN_WIDTH, WIN_HEIGHT,
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if(g_pWindow == NULL)
    {
        fprintf(stderr, "Create window failed, %s\n", SDL_GetError());
        goto __END;
    }

    g_pRenderer = SDL_CreateRenderer(g_pWindow, -1, 0);
    if(g_pRenderer == NULL)
    {
        fprintf(stderr, "Create Renderer failed, %s\n", SDL_GetError());
        goto __END;
    }

    if(PlayerInit(input_file_path, pAVglobal) != 0)
    {
        av_log(NULL, AV_LOG_FATAL, "Init avplayer failed!\n");
    }

    SdlEventLoop(pAVglobal);

__END:

    return 0;
}

int PlayerInit(const char* pFileName, AVGlobal* pAVglobal)
{
    if(nullptr == pFileName)
    {
        return -1;
    }

    if(nullptr == pAVglobal)
    {
        return -2;
    }

    //初始化解码前音频包队列
    if(pAVglobal->m_queAudioPacket.InitPacketQueue() < 0 ||
            pAVglobal->m_queVideoPacket.InitPacketQueue() < 0)
    {
        return -2;
    }

    //初始化解码后视频帧队列
    if(pAVglobal->m_videoFrameQueue.InitVideoFrameQueue(VIDEO_PICTURE_QUEUE_SIZE) < 0)
    {
        return -3;
    }

    pAVglobal->m_pFileName = (char *)pFileName;
    //以音频为主，去做音视频同步
    pAVglobal->SetAudioVideoSyncType(AV_SYNC_AUDIO_MASTER);

    pAVglobal->m_pReadThread = SDL_CreateThread(ReadAVDataThread, "read_thread", pAVglobal);
    if(pAVglobal->m_pReadThread == NULL)
    {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateThread:%s\n", SDL_GetError());
        return -4;
    }

    ScheduleRefresh(pAVglobal, 40);

    return 0;
}

void SdlEventLoop(AVGlobal *pAVGlobal)
{
    SDL_Event event;
    for(;;)
    {
        SDL_WaitEvent(&event);
        switch (event.type)
        {
            case SDL_QUIT:
                pAVGlobal->m_bQuit = true;
                break;
            case FF_REFRESH_EVENT:
                VideoFrameRefresh(pAVGlobal);
                break;
            default:
                break;
        }
    }
}

void ScheduleRefresh(AVGlobal *pGlobal, int timerLen)
{
    SDL_AddTimer(timerLen, SdlRefreshTimerCallBack, pGlobal);
}

Uint32 SdlRefreshTimerCallBack(Uint32 interval, void *arg)
{
    SDL_Event event;
    event.type = FF_REFRESH_EVENT;
    event.user.data1 = arg;
    SDL_PushEvent(&event);
    return 0;
}

void VideoFrameRefresh(void *arg)
{
    AVGlobal *pAVGlobal = (AVGlobal *)arg;
    if(NULL == pAVGlobal)
    {
        return;
    }

    VideoFrame *pvFrame = NULL;

    if(pAVGlobal->m_pStreamVideo)
    {
        if(pAVGlobal->m_videoFrameQueue.m_nCountEle == 0)
        {
            ScheduleRefresh(pAVGlobal, 1);    //队列中没有解码后的帧时，需要尽快多次刷新。
        }
        else
        {
            pvFrame = pAVGlobal->m_videoFrameQueue.FrameQueuePeek();
            pAVGlobal->m_video_current_pts_time = pvFrame->pts;
        }
    }
    else
    {
        ScheduleRefresh(pAVGlobal, 100);   //还未解析到视频流，再等待
    }
}

void RenderPicture(AVGlobal *pGlobal)
{
    VideoFrame *pv = pGlobal->m_videoFrameQueue.FrameQueuePeek();

    SDL_Rect rect;
    rect.w = 200;
    rect.h = 200;
    rect.x = 500;
    rect.y = 500;

    if(pGlobal->m_pTextTure == NULL)
    {
        Uint32 pixformat = SDL_PIXELFORMAT_IYUV;   //以YUV格式渲染
        pGlobal->m_pTextTure = SDL_CreateTexture(g_pRenderer, pixformat,
                                                 pv->nWidth, pv->nHight,
                                                 SDL_TEXTUREACCESS_STREAMING);

        SDL_RenderClear(g_pRenderer);
        SDL_RenderCopy(g_pRenderer, pGlobal->m_pTextTure, NULL, &rect);
        SDL_RenderPresent(g_pRenderer);
    }

    pGlobal->m_videoFrameQueue.FrameQueuePop();
}