#ifndef VIDEOSTREAMCAPTURE_H
#define VIDEOSTREAMCAPTURE_H

#include <QObject>
#include <QThread>
#include <QDateTime>
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

};



struct VideoCaptureInfo
{
public:
    QString rtmpAddr; //推流地址
    int frameRate = 0; //帧率
    int bitRate = 0; //比特率

    //桌面录制
    int desktopRange_x = 0; //录制区域的起始坐标-x
    int desktopRange_y = 0; //录制区域的起始坐标-y
    int desktopWidth = 0; //图像宽度
    int desktopHeight = 0; //图像高度

    //摄像头录制
    QString cameraName;
    int cameraIndex = 0;
    int cameraWidth = 0; //图像宽度
    int cameraHeight = 0; //图像高度
    int cameraPos_x = 0; //显示区域的起始坐标-x
    int cameraPos_y = 0; //显示区域的起始坐标-y
};


class VideoStreamCapture : public QThread
{
    Q_OBJECT
public:
    VideoStreamCapture(QObject* parent = nullptr);
    ~VideoStreamCapture();

private:
    int DesktopCaptureInit(); //屏幕采集初始化
    int CameraCaptureInit(); //摄像头采集初始化
    int VideoFilterInit(); //视频滤镜初始化,用于视频合流
    int H264RecodeInit(); //H264编码器初始化
    int OutPutInit(); //输出初始化

    int MergeCaptureStart(); //开始推流-桌面+摄像头
    int GetDesktopFrame(AVPacket* packet, AVFrame* pFrame, AVFrame* pFrameYUV, SwsContext* pImgConvertCtx); //获取屏幕采集Frame
    int GetCameraFrame(AVPacket* packet, AVFrame* pFrame, AVFrame* pFrameYUV, SwsContext* pImgConvertCtx); //获取摄像头采集Frame

public:
    int Init(); //初始化
    void CaptureStart(); //开始推流
    void CaptureStop(); //停止推流

signals:
    void Stop();

public slots:
    void onCaptureStop();

protected:
    virtual void run() override;

private:

    VideoCaptureInfo* pCaptureInfo = NULL; //推流配置信息
/********************桌面捕捉***********************/
    AVFormatContext* pDesktopInFormatCtx = NULL;
    const AVCodec* pDesktopCodec = NULL;
    AVCodecContext* pDesktopCodecCtx = NULL;

/********************摄像头捕捉***********************/
    AVFormatContext* pCameraInFormatCtx = NULL;
    const AVCodec* pCameraCodec = NULL;
    AVCodecContext* pCameraCodecCtx = NULL;

/********************滤镜***********************/
    AVFilterContext** pVideoBuffersrcCtx = NULL;
    AVFilterContext* pVideoBuffersinkCtx = NULL;

/********************输出***********************/
    AVFormatContext* pOutFormatCtx = NULL;
    const AVCodec* pH264Codec = NULL;
    AVCodecContext* pH264CodecCtx= NULL;

    int desktopStreamIndex = -1; //桌面视频流索引
    int cameraStreamIndex = -1; //摄像头视频流索引
    int outStreamIndex = -1; //输出视频流索引

    bool isRunning = true;

};

#endif // VIDEOSTREAMCAPTURE_H
