*. 基于FFmpeg + SDL 的音视频播放器

*. 由于STL中的容器是非线程安全的，因为要实现解封装和解码的并行操作需要实现一个线程安全的队列

*. ffmpeg的一些结构体说明
AVFifo:
AVFormatContext:
AVCodecContext:
AVStream:

AVPacket 和 AVFrame是采用引用计数的方式进行内部资源的管理
AVPacket: 解码前的数据
AVFrame: 解码后的数据

