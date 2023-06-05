#include "AVCallBackFunc.h"
#include "AVGlobal.h"

/* stream 指向音频数据的buffer
 * len: 设备想要多少数据字节数
 * */

void CallBackSdlAudio(void *userdata, Uint8 *stream, int lenDeviceNeed)
{
    AVGlobal *pGlobal = (AVGlobal *)userdata;
    if(NULL == pGlobal)
    {
        return;
    }

    int lenCurBuffer = 0;  //目前队列中还剩的数据
    int lenCodec = 0;      //当次解码的数据的长度

    while(lenDeviceNeed > 0)
    {
        //先判断解码后的PCM数据buffer是否还有数据
        if(pGlobal->m_audio_buff_use_pos >= pGlobal->m_audio_buff_size)
        {
            //已经没有数据，需要进行解码packet
            lenCodec = pGlobal->DecodeAudioPacket();
            if(lenCodec > 0)
            {
                pGlobal->m_audio_buff_size = lenCodec;
            }
            else  //没有数据或者出错，则补静音数据
            {
                pGlobal->m_audio_buff_size = SDL_AUDIO_BUFFER_SIZE;   //默认补1024B
                pGlobal->m_pcmbuffer = NULL;
            }

            pGlobal->m_audio_buff_use_pos = 0;
        }

        //获取当前解码队列中还剩多少数据
        lenCurBuffer = pGlobal->m_audio_buff_size - pGlobal->m_audio_buff_use_pos;
        if(lenCurBuffer > lenDeviceNeed)
        {
            lenCurBuffer = lenDeviceNeed;
        }

        if (pGlobal->m_pcmbuffer != NULL)
        {
            memcpy(stream, (uint8_t *)pGlobal->m_pcmbuffer + pGlobal->m_audio_buff_use_pos, lenCurBuffer);
        }
        else
        {
            memset(stream, 0, lenCurBuffer);  //补1024个B的静音数据给声卡
        }

        lenDeviceNeed -= lenCurBuffer;
        stream += lenCurBuffer;
        pGlobal->m_audio_buff_use_pos += lenCurBuffer;
    }
}

int VideoDecodeThread(void *arg)
{
    int nRet = -1;
    for(;;)
    {

    }
}
