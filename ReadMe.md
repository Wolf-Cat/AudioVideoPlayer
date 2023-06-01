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
    AVCodecContext: 编解码上下文
    AVCodec: 编解码器

AVPacket: 包，解码前的数据，音视频压缩的数据
AVFrame: 帧，解码后的数据，音视频解压后的原始数据，视频对应的是YUV，音频对应的是PCM

AVPacket 和 AVFrame是采用引用计数的方式进行内部资源的管理

*. 由于STL中的容器是非线程安全的，因为要实现解封装和解码的并行操作需要实现一个线程安全的队列
