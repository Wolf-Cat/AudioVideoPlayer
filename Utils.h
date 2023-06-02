#ifndef AUDIOVIDEOPLAYER_UTILS_H
#define AUDIOVIDEOPLAYER_UTILS_H

extern "C" {
    #include <libavutil/error.h>
    #include <string.h>    //memset的头文件
};

char* my_av_err2str(int errnum)
{
    static char str[AV_ERROR_MAX_STRING_SIZE];
    memset(str, 0, sizeof(str));
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}

#endif //AUDIOVIDEOPLAYER_UTILS_H
