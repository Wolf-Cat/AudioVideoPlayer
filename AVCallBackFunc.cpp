#include "AVCallBackFunc.h"
#include "Utils.h"
/* stream 指向音频数据的buffer
 * len: 设备想要多少数据字节数
 * */

#define FF_QUIT_EVENT (SDL_USEREVENT + 1)

int ReadAVDataThread(void *arg)
{
    av_log(NULL, AV_LOG_DEBUG, "Enter ReadAVDataThread\n");
    int nRet = -1;
    AVPacket *pktTmp = av_packet_alloc();

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

    if(pAVglobal->m_nVideoIndex >= 0)
    {
        pAVglobal->GetStreamComponent(pAVglobal->m_nVideoIndex);
    }

    for (;;) {
        if(pAVglobal->m_bQuit)
        {
            nRet = -1;
            goto __ERROR;
        }

        //开始读取AVPacket
        nRet = av_read_frame(pAVglobal->m_pAVformatContext, pktTmp);
        if(nRet < 0)
        {
            if(pAVglobal->m_pAVformatContext->pb->error == 0)
            {
                SDL_Delay(100);
                continue;
            }
            else
            {
                break;
            }
        }

        //将AVPacket送到放入对应的音频/视频的 AVPacket队列中
        if(pktTmp->stream_index == pAVglobal->m_nAudioIndex)
        {
            pAVglobal->m_queAudioPacket.PushPacketQueue(pktTmp);
        }
        else if(pktTmp->stream_index == pAVglobal->m_nVideoIndex)
        {
            pAVglobal->m_queVideoPacket.PushPacketQueue(pktTmp);
        }
        else  //暂且不处理字幕流或其他流的包
        {
            av_packet_unref(pktTmp);
        }
    }

    while (pAVglobal->m_bQuit)
    {
        SDL_Delay(100);
    }

    nRet = 0;

    __ERROR:
    if(pktTmp != NULL)
    {
        av_packet_free(&pktTmp);
    }

    if(nRet != 0)
    {
        SDL_Event event;
        event.type = FF_QUIT_EVENT;
        event.user.data1 = pAVglobal;
        SDL_PushEvent(&event);
    }

    return nRet;
}

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
    AVGlobal *pGlobal = (AVGlobal *)arg;
    if(pGlobal == NULL)
    {
        return nRet;
    }

    AVFrame *pvFrame = av_frame_alloc();
    double durationCurFrame = 0;
    double ptsSecond = 0;
    double ptsTime = 0;
    //获取帧率，一秒钟多少张图, //num = 25(分子), den = 1（分母），每秒25帧;
    AVRational vfps = av_guess_frame_rate(pGlobal->m_pAVformatContext, pGlobal->m_pStreamVideo, NULL);

    //num = 1, den = 90000, AStream里面的time_base是单元时间。
    //每种格式的time_base的值不一样，根据采样来计算，比如mpeg的pts、dts都是以90kHz来采样的，所以采样间隔就是1/900000秒。
    //时基， 通过time_base可以把dts/pts转化为真正的时间。ffmpeg其他结构体中也有这个字段，只有AVStream中的time_base*pts=真正的时间（雷神）
    AVRational vtime_base = pGlobal->m_pStreamVideo->time_base;

    for(;;)
    {
        if(pGlobal->m_bQuit)
        {
            break;
        }

        if(pGlobal->m_queVideoPacket.GetPacketQueueElement(&pGlobal->m_videoPkt, false))
        {
            av_log(pGlobal->m_pCodecCtxVideo, AV_LOG_DEBUG, "Get a video packet, delay 10ms");
            SDL_Delay(10);
            continue;
        }

        //将视频包送到视频解码器
        nRet = avcodec_send_packet(pGlobal->m_pCodecCtxVideo, &pGlobal->m_videoPkt);
        av_packet_unref(&pGlobal->m_videoPkt);
        if(nRet < 0)
        {
            av_log(pGlobal->m_pCodecCtxVideo, AV_LOG_ERROR, "Failed to send pkt to video decoder!\n");
            goto __ERROR;
        }

        while(nRet >= 0)
        {
            nRet = avcodec_receive_frame(pGlobal->m_pCodecCtxVideo, pvFrame);
            if(nRet == AVERROR(EAGAIN) || nRet == AVERROR_EOF)
            {
                break;
            }
            else if(nRet < 0)
            {
                av_log(pGlobal->m_pCodecCtxVideo, AV_LOG_ERROR, "Failed to receive frame from video decoder!\n");
                nRet = -1;
                goto __ERROR;
            }

            //音视频同步
            //1. 首先计算这一帧的播放时长是多少
            durationCurFrame = (vfps.num && vfps.den ? av_q2d((AVRational){vfps.den, vfps.num}): 0);
            //2. 将pts转化为以秒为单位的时间
            ptsSecond = (pvFrame->pts == AV_NOPTS_VALUE) ? NAN : pvFrame->pts * av_q2d(vtime_base);
            //3. 计算视频的clock时长到哪了
            ptsSecond = SynchronizeVideoPts(pGlobal, pvFrame, ptsSecond);

            av_frame_unref(pvFrame);
        }
    }

    __ERROR:
    av_frame_free(&pvFrame);
    return nRet;
}

double SynchronizeVideoPts(AVGlobal *pGlobal, AVFrame *frame_src, double pts)
{
    if(pGlobal == NULL)
    {
        return -1;
    }

    double frame_delay_uint_time;     //单位时间
    if(pts != 0)
    {
        pGlobal->m_clock_video = pts;
    }
    else
    {
        pts = pGlobal->m_clock_video;
    }

    frame_delay_uint_time = av_q2d(pGlobal->m_pCodecCtxVideo->time_base);
    //如果有重复帧
    frame_delay_uint_time += frame_src->repeat_pict * (frame_delay_uint_time * 0.5);
    pGlobal->m_clock_video += frame_delay_uint_time;

    return pts;
}
