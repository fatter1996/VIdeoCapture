#ifndef CAPTIONSCOMPOSE_H
#define CAPTIONSCOMPOSE_H

#include <QObject>

//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavdevice/avdevice.h"
#include <libavutil/time.h>
#include "libavutil/mathematics.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/avutil.h>
#include "libavutil/audio_fifo.h"

};


class CaptionsCompose
{
    //Q_OBJECT
public:
    CaptionsCompose();
    ~CaptionsCompose();

public:
    int Init();
    int ComposeStart(); //开始合成字幕

private:
    int VideoInit(QString inPutPath); //视频输入初始化
    int VideoStreamInit();
    int AudioStreamInit();

    int H264RecodeInit(); //H264编码器初始化
    int AudioRecodeInit(); //音频编码器初始化

    int VideoFilterInit(); //视频滤镜初始化,用于字幕合成

    int OutPutInit(QString outPutPath); //输出初始化
signals:
    void StopCompose();


private:
    bool isRunning = true;
    QString fileName; //保存的视频文件名称

/********************输入***********************/
    AVFormatContext* pInFormatCtx = NULL; //输入句柄
    const AVCodec* pVideoCodec = NULL; //视频解码器
    AVCodecContext* pVideoCodecCtx = NULL; //视频解码器句柄
    const AVCodec* pAudioCodec = NULL; //音频解码器
    AVCodecContext* pAudioCodecCtx = NULL;  //音频解码器句柄

/*********************滤镜************************/
    AVFilterContext** pVideoBuffersrcCtx = NULL;
    AVFilterContext* pVideoBuffersinkCtx = NULL;

/*********************输出**********************/
    AVFormatContext* pOutFormatCtx = NULL; //输出上下文
    const AVCodec* pOutVideoCodec = NULL; //视频编码器
    AVCodecContext* pOutVideoCodecCtx = NULL; //视频编码器句柄
    const AVCodec* pOutAudioCodec = NULL;  //音频编码器
    AVCodecContext* pOutAudioCodecCtx = NULL; //音频编码器句柄
/********************缓冲***********************/
    AVFifoBuffer* pVideoFifo = NULL; //视频缓冲
    AVAudioFifo* pAudioFifo = NULL; //音频缓冲

/********************流索引***********************/
    int videoIndex = -1; //视频流索引
    int audioIndex = -1; //音频流索引
    int outVideoIndex = -1; //视频输出流索引
    int outAudioIndex = -1; //音频输出流索引

};

#endif // CAPTIONSCOMPOSE_H
