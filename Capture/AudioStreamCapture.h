#ifndef AUDIOSTREAMCAPTURE_H
#define AUDIOSTREAMCAPTURE_H

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


struct AudioCaptureInfo
{
public:
    QString rtmpAddr; //推流地址
    int SampleRate = 0; //帧率
    int bitRate = 0; //比特率

    QString MICName; //设备名称
    int MICIndex = 0; //设备编号
};

class AudioStreamCapture : public QThread
{
    Q_OBJECT
public:
    AudioStreamCapture(QObject* parent = nullptr);
    ~AudioStreamCapture();

private:
    int MICCaptureInit(); //麦克风采集初始化
    int AudioFilterInit(); //音频滤镜初始化,用于重采样
    int AudioRecodeInit(); //音频编码器初始化
    int OutPutInit(); //输出初始化
    int MICCaptureStart(); //开始推流-麦克风

public:
    int Init(); //初始化
    void CaptureStart(); //开始推流
    void CaptureStop(); //停止推流

signals:
    void StopAudio();

public slots:
    void onCaptureStop();

protected:
    virtual void run() override;

private:

AudioCaptureInfo* pCaptureInfo = NULL; //推流配置信息
/********************麦克风捕捉***********************/
    AVFormatContext* pMICInFormatCtx = NULL;
    const AVCodec* pMICCodec = NULL;
    AVCodecContext* pMICCodecCtx = NULL;

/********************滤镜***********************/
    AVFilterContext* pAudioBuffersrcCtx = NULL;
    AVFilterContext* pAudioBuffersinkCtx = NULL;

/********************输出***********************/
    AVFormatContext* pOutFormatCtx = NULL;
    const AVCodec* pAudioCodec = NULL;
    AVCodecContext* pAudioCodecCtx= NULL;

    int audioStreamIndex = -1; //麦克风音频流索引
    int outStreamIndex = -1; //输出音频流索引

    bool isRunning = true;
};

#endif // AUDIOSTREAMCAPTURE_H
