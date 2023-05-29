#include <iostream>
extern "C" {
    #include <libavutil/log.h>
    #include <libavformat/avformat.h>
    #include <libavutil/error.h>
    #include <SDL2/SDL.h>
}
#include "AVGlobal.h"

using namespace std;

namespace {
    const int WIN_WIDTH = 640;
    const int WIN_HEIGHT = 480;
}

static SDL_Window *g_pWindow = NULL;
static SDL_Renderer *g_pRenderer = NULL;

int PlayerInit(const char* pFileName, AVGlobal *pAVGlobal);


int main(int argc, char *agrv[])
{
    av_log_set_level(AV_LOG_DEBUG);
    av_log(NULL, AV_LOG_INFO, "Hello ffmpeg\n");

    uint32_t version = avformat_version();
    cout << "version:" << version << endl;

    char input_file_path[] = "./testVedio.mp4";
    int nRet = -1;
    SDL_Texture *pTextTure = NULL;
    AVGlobal* pAVglobal = new AVGlobal;
    AVFormatContext *pFmtContex = NULL;
    AVStream *pInstream = NULL;
    int nIdx = 0;

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

    if(pAVglobal->m_queAudio.InitPacketQueue() < 0 ||
            pAVglobal->m_queVedio.InitPacketQueue() < 0)
    {
        return -2;
    }

    return 0;
}