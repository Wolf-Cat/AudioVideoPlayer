#include <iostream>
extern "C" {
    //#include <libavutil/log.h>
    //#include <SDL.h>
}

using namespace std;

//SDL_Window *g_pWin = NULL;
//SDL_Renderer *g_pRenderer = NULL;

int main(int argc, char *agrv[])
{
    char fileAvPath[10] = "./mp4";
    int a = 0;
    a = 1;
    int k  = 8999;
    std::cout << "hello wiord" << std::endl;
    //av_log_set_level(AV_LOG_DEBUG);
    //av_log(NULL, AV_LOG_INFO, "Hello ffmpeg");
    std::cout << "hello wiordGFFFFFFFFddddddd" << std::endl;
    
    //if(SDL_Init(SDL_INIT_EVERYTHING) == 0)
    {
        std::cout << "SDL init success" << std::endl;
    }

    return 0;
}