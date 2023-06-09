#ifndef AUDIOVIDEOPLAYER_AVCALLBACKFUNC_H
#define AUDIOVIDEOPLAYER_AVCALLBACKFUNC_H

extern "C" {
    #include "SDL2/SDL.h"
};

#include "AVGlobal.h"

#define SDL_AUDIO_BUFFER_SIZE 1024

int ReadAVDataThread(void *arg);        //读取音视频数据线程
void CallBackSdlAudio(void *userdata, Uint8 *stream, int lenDeviceNeed);
int VideoDecodeThread(void *arg);

double SynchronizeVideoPts(AVGlobal *pGlobal, AVFrame *frame_src, double pts);

#endif //AUDIOVIDEOPLAYER_AVCALLBACKFUNC_H
