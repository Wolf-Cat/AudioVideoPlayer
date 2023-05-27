#include <iostream>
extern "C" {
    #include <libavutil/log.h>
    #include <libavformat/avformat.h>
    #include <SDL2/SDL.h>
}

using namespace std;

SDL_Window *g_pWin = NULL;
SDL_Renderer *g_pRenderer = NULL;

int main(int argc, char *agrv[])
{
    char fileAvPath[10] = "./mp4";
    std::cout << "hello wiord" << std::endl;
    av_log_set_level(AV_LOG_DEBUG);
    av_log(NULL, AV_LOG_INFO, "Hello ffmpeg");

    uint32_t version = avformat_version();
    cout << "version:" << version << endl;

    if(SDL_Init(SDL_INIT_EVERYTHING) == 0)
    {
        std::cout << "SDL init success" << std::endl;
    }

    return 0;
}