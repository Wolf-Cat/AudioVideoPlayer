*. 基于FFmpeg + SDL 的音视频播放器

*. 播放器的线程：
 （1）主线程
 （2）read_thread：专门用于从多媒体文件中读取音视频包
 （3）decode_thread：用于处理视频解码的线程
 （4）设置扬声器的时候由系统自动分配的, 在SDL_OpenAudio函数中创建

*. ffmpeg的一些结构体说明
AVFifo:
AVFormatContext: 存储音视频封装格式中包含的信息
    AVStream: 存储音频/视频流信息
        AVCodecParameters: 包含AVCodecContext解码结构体的结构体参数
            AVCodecID: 解码器ID, AV_CODEC_ID_H264, AV_CODEC_ID_AAC等等

 
AVCodecContext: 编解码器上下文
    AVCodec: 编解码器  
    AVCodecID

AVPacket: 包，解码前的数据，音视频压缩的数据
AVFrame: 帧，解码后的数据，音视频解压后的原始数据，视频对应的是YUV，音频对应的是PCM

AVPacket 和 AVFrame是采用引用计数的方式进行内部资源的管理

SwrContext: 音频重采样上下文

*. 一些函数说明
avformat_find_stream_info()
会读取一段视频文件数据并尝试解码，将取到的流信息填入AVFormatContext.streams中。
AVFormatContext.streams是一个指针数组，数组大小是AVFormatContext.nb_streams

//将编码的数据包发送给编解码器进行解码
avcodec_send_packet(AVCodecContext *avctx, const AVPacket *pkt);

//从编解码器的输出队列中获取解码后的输出帧。frame为输出参数
avcodec_receive_frame(AVCodecContext *avctx, AVFrame *frame)

av_read_frame()
看上去命名像都的是帧，其实读出来的是压缩编码的AVPacket
对于视频来说，一个packet只包含一个视频帧
对于音频来说，若是帧长固定的格式则一个packet可包含整数个音频帧，如果帧长可变的格式则一个packet只包含一个音频帧
读取到的packet每次使用完之后应调用av_packet_unref()清空，否则会造成内存泄露

*. 由于STL中的容器是非线程安全的，因为要实现解封装和解码的并行操作需要实现一个线程安全的队列
