*. 基于FFmpeg + SDL 的音视频播放器

*. 播放器的线程：
 （1）主线程
 （2）read_thread：专门用于从多媒体文件中读取音视频包
 （3）decode_thread：用于处理视频解码的线程
 （4）设置扬声器的时候由系统自动分配的, 在audio_open函数中

*. ffmpeg的一些结构体说明
AVFifo:
AVFormatContext:
AVCodecContext:
AVStream:

AVPacket 和 AVFrame是采用引用计数的方式进行内部资源的管理
AVPacket: 解码前的数据
AVFrame: 解码后的数据

*. 由于STL中的容器是非线程安全的，因为要实现解封装和解码的并行操作需要实现一个线程安全的队列
