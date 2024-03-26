#ifndef DESKTOPCAPTURE_H
#define DESKTOPCAPTURE_H
#include <QObject>
#include <QThread>
#include <QFile>
#include "Universal.h"
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

class Capture : public QThread
{
    Q_OBJECT
public:
    Capture(QObject* parent = nullptr);
    ~Capture();

public:
    void Init();
    int CaptureStart(); //开始推流
    QString CaptureStop(); //停止推流

private:
    int DesktopCaptureInit(); //屏幕采集初始化
    int CameraCaptureInit(); //摄像头采集初始化
    int MICCaptureInit(); //麦克风采集初始化

    int H264RecodeInit(); //H264编码器初始化
    int AudioRecodeInit(); //音频编码器初始化

    int VideoFilterInit(); //视频滤镜初始化,用于视频合流
    int AudioFilterInit(); //音频滤镜初始化,用于重采样

    int OutPutInit(); //输出初始化

    int DesktoCaptureStart(); //开始推流-桌面
    int MergeCaptureStart(); //开始推流-桌面+摄像头
    int MICCaptureStart(); //开始推流-麦克风

    int GetDesktopFrame(AVPacket* pPacket, AVFrame* pFrame, AVFrame* pFrameYUV, SwsContext* pSwsCtx); //获取屏幕采集Frame
    int GetCameraFrame(AVPacket* pPacket, AVFrame* pFrame, AVFrame* pFrameYUV, SwsContext* pSwsCtx); //获取摄像头采集Frame

signals:
    void Error();

protected:
    virtual void run() override;

private:
    static int interrupt_callback(void *p); //超时回调

private:
    bool isRunning = true;
    int initFlag = 0; //监控初始化状态
    int flag = 0;
    QString fileName; //保存的视频文件名称
    static Runner input_runner;
    //QFile* log = nullptr;
/********************桌面捕捉***********************/
    AVFormatContext* pDesktopInFormatCtx = NULL; //桌面输入句柄
    const AVCodec* pDesktopCodec = NULL; //桌面解码器
    AVCodecContext* pDesktopCodecCtx = NULL; //桌面解码器句柄

/********************摄像头捕捉***********************/
    AVFormatContext* pCameraInFormatCtx = NULL; //摄像头输入句柄
    const AVCodec* pCameraCodec = NULL; //摄像头解码器
    AVCodecContext* pCameraCodecCtx = NULL; //摄像头解码器句柄

/********************麦克风捕捉***********************/
    AVFormatContext* pMICInFormatCtx = NULL; //麦克风输入句柄
    const AVCodec* pMICCodec = NULL; //麦克风解码器
    AVCodecContext* pMICCodecCtx = NULL;  //麦克风解码器句柄

/*********************滤镜************************/
    AVFilterContext** pVideoBuffersrcCtx = NULL;
    AVFilterContext* pVideoBuffersinkCtx = NULL;
    AVFilterContext* pAudioBuffersrcCtx = NULL;
    AVFilterContext* pAudioBuffersinkCtx = NULL;

/*********************输出**********************/
    AVFormatContext* pOutFormatCtx = NULL; //输出上下文
    const AVCodec* pH264Codec = NULL; //H264视频编码器
    AVCodecContext* pH264CodecCtx= NULL; //H264视频编码器句柄
    const AVCodec* pAudioCodec = NULL; //aac音频编码器
    AVCodecContext* pAudioCodecCtx= NULL; //aac音频编码器句柄

/********************缓冲***********************/
    AVFifoBuffer* pVideoFifo = NULL; //视频缓冲
    AVAudioFifo* pAudioFifo = NULL; //音频缓冲

/********************流索引***********************/
    int videoIndex = -1; //桌面视频流索引
    int videoIndexC = -1; //摄像头视频流索引
    int audioIndex = -1; //麦克风音频流索引
    int outVideoIndex = -1; //视频输出流索引
    int outAudioIndex = -1; //音频输出流索引

};

#endif // DESKTOPCAPTURE_H
