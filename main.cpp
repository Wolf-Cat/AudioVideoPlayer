#include <iostream>
extern "C" {
    #include <libavutil/log.h>
    #include <libavformat/avformat.h>
    #include <SDL2/SDL.h>
}

using namespace std;

namespace {
    const int WIN_WIDTH = 640;
    const int WIN_HEIGHT = 480;
}

static SDL_Window *g_pWindow = NULL;
static SDL_Renderer *g_pRenderer = NULL;

int main(int argc, char *agrv[])
{
    av_log_set_level(AV_LOG_DEBUG);
    av_log(NULL, AV_LOG_INFO, "Hello ffmpeg\n");

    uint32_t version = avformat_version();
    cout << "version:" << version << endl;

    char fileAvPath[10] = "./mp4";
    int nRet = -1;
    SDL_Texture *pTextTure = NULL;
    AVFormatContext *pFmtContex = NULL;

    if(SDL_Init(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == 0))
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

    //打开多媒体文件，并获得流信息
    nRet = avformat_open_input(&pFmtContex, fileAvPath, NULL, NULL);
    if(nRet < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "%s\n", av_err2str(nRet));
        goto __END;
    }

    nRet = avformat_find_stream_info(pFmtContex, NULL);
    if(nRet < 0){
        av_log(NULL, AV_LOG_ERROR, "%s\n", av_err2str(nRet));
        goto __END;
    }

    __END:

    return 0;
}