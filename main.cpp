#include <iostream>
extern "C" {
    #include <libavutil/log.h>
    #include <libavformat/avformat.h>
    #include <libavutil/error.h>
    #include <SDL2/SDL.h>
}
#include "AVGlobal.h"
#include "Utils.h"

using namespace std;

namespace {
    const int WIN_WIDTH = 640;
    const int WIN_HEIGHT = 480;
}

static SDL_Window *g_pWindow = NULL;
static SDL_Renderer *g_pRenderer = NULL;

int PlayerInit(const char* pFileName, AVGlobal *pAVGlobal);
int ReadAVDataThread(void *arg);        //读取音视频数据线程

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
            pAVglobal->m_queVedio.InitPacketQueue() < 0)
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

    SDL_Delay(5000);   //防止主线程马上退出，无法执行子线程
    return 0;
}

int ReadAVDataThread(void *arg)
{
    av_log(NULL, AV_LOG_DEBUG, "Enter ReadAVDataThread\n");
    int nRet = -1;
    AVPacket *pktTmp = NULL;

    AVGlobal *pAVglobal = (AVGlobal *)arg;
    if(arg == NULL || pAVglobal == NULL) {
        goto __ERROR;
    }

    //打开媒体文件，获取到媒体信息上下文AVFormatContext
    nRet = avformat_open_input(&pAVglobal->m_pAVformatContext, pAVglobal->m_pFileName, NULL, NULL);
    if(nRet < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Can not open file:%s, %d, %s\n", pAVglobal->m_pFileName, nRet, my_av_err2str(nRet));
        goto __ERROR;
    }

    //找到音频流, 视频流, (或者字幕流)的流索引，AVStream **streams
    for(int i = 0; i < pAVglobal->m_pAVformatContext->nb_streams; ++i)
    {
        AVStream *pSt = pAVglobal->m_pAVformatContext->streams[i];
        enum AVMediaType eType = pSt->codecpar->codec_type;
        if(eType == AVMEDIA_TYPE_AUDIO && pAVglobal->m_nAudioIndex == -1)
        {
            pAVglobal->m_nAudioIndex = i;
        }
        if(eType == AVMEDIA_TYPE_VIDEO && pAVglobal->m_nVideoIndex == -1)
        {
            pAVglobal->m_nVideoIndex = i;
        }

        //已经找到视频和音频的流索引
        if(pAVglobal->m_nAudioIndex >= 0 && pAVglobal->m_nVideoIndex >= 0)
        {
            av_log(NULL, AV_LOG_DEBUG, "already find audioIndex:%d , videoIndex:%d", pAVglobal->m_nAudioIndex, pAVglobal->m_nVideoIndex);
            break;
        }
    }

    if(pAVglobal->m_nAudioIndex == -1 || pAVglobal->m_nVideoIndex == -1)
    {
        av_log(NULL, AV_LOG_ERROR, "find audioIndex:%d , videoIndex:%d error", pAVglobal->m_nAudioIndex, pAVglobal->m_nVideoIndex);
        goto __ERROR;
    }

    if(pAVglobal->m_nAudioIndex >= 0)
    {
        pAVglobal->GetStreamComponent(pAVglobal->m_nAudioIndex);
    }

    __ERROR:
    return nRet;
}