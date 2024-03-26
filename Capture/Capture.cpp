#include "Capture.h"
#include <QDebug>
#include <QSettings>
#include <QCameraInfo>
#include <QAudioDeviceInfo>
#include <QtConcurrent>
#include <windows.h>
#include "Universal.h"

Runner Capture::input_runner = {0};

Capture::Capture(QObject* parent)
{
    avdevice_register_all(); //注册设备
    avformat_network_init(); //加载socket库以及网络加密协议相关的库
    //log = new QFile("log.txt");

}

Capture::~Capture()
{
    /* close input */
    if (pDesktopInFormatCtx)
        avformat_close_input(&pDesktopInFormatCtx);
    if (pCameraInFormatCtx)
        avformat_close_input(&pCameraInFormatCtx);
    if (pMICInFormatCtx)
        avformat_close_input(&pMICInFormatCtx);

    /* close codec context */
    if(pDesktopCodecCtx)
        avcodec_close(pDesktopCodecCtx);
    if(pCameraCodecCtx)
        avcodec_close(pCameraCodecCtx);
    if(pMICCodecCtx)
        avcodec_close(pMICCodecCtx);
    if(pH264CodecCtx)
        avcodec_close(pH264CodecCtx);
    if(pAudioCodecCtx)
        avcodec_close(pAudioCodecCtx);

    if(pVideoBuffersrcCtx)
    {
        avfilter_free(pVideoBuffersrcCtx[0]);
        avfilter_free(pVideoBuffersrcCtx[1]);
    }
    if(pVideoBuffersinkCtx)
        avfilter_free(pVideoBuffersinkCtx);
    if(pAudioBuffersrcCtx)
        avfilter_free(pAudioBuffersrcCtx);
    if(pAudioBuffersinkCtx)
        avfilter_free(pAudioBuffersinkCtx);

    /* close output */
    if (pOutFormatCtx)
    {
        if (!(pOutFormatCtx->oformat->flags & AVFMT_NOFILE))
            avio_close(pOutFormatCtx->pb);
        avformat_free_context(pOutFormatCtx);
    }

    //log->close();
}

void Capture::Init()
{
    //屏幕采集初始化
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init] DesktopCaptureInit\n");
    //log->close();

    if(!(initFlag & 0x01) && DesktopCaptureInit() < 0)
    {
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("[Init Error] DesktopCaptureInit\n");
        //log->close();
        if (pDesktopInFormatCtx)
            avformat_close_input(&pDesktopInFormatCtx);
        if(pDesktopCodecCtx)
            avcodec_close(pDesktopCodecCtx);

        return Init();
    }

    //摄像头采集初始化
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init] CameraCaptureInit\n");
    //log->close();
    if(!(initFlag & 0x02) && CameraCaptureInit() < 0)
    {
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("[Init Error] CameraCaptureInit\n");
        //log->close();
        if (pCameraInFormatCtx)
            avformat_close_input(&pCameraInFormatCtx);
        if(pCameraCodecCtx)
            avcodec_close(pCameraCodecCtx);

        return Init();
    }


    //麦克风采集初始化
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init] MICCaptureInit\n");
    //log->close();
    if(!(initFlag & 0x04) && MICCaptureInit() < 0)
    {
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("[Init Error] MICCaptureInit\n");
        //log->close();
        if (pMICInFormatCtx)
            avformat_close_input(&pMICInFormatCtx);
        if(pMICCodecCtx)
            avcodec_close(pMICCodecCtx);

        return Init();
    }


    //H264编码器初始化
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init] H264RecodeInit\n");
    //log->close();
    if(!(initFlag & 0x08) && H264RecodeInit() < 0)
    {
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("[Init Error] H264RecodeInit\n");
        //log->close();
        if(pH264CodecCtx)
            avcodec_close(pH264CodecCtx);

        return Init();
    }

    //音频编码器初始化
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init] AudioRecodeInit\n");
    //log->close();
    if(!(initFlag & 0x10) && AudioRecodeInit() < 0)
    {
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("[Init Error] AudioRecodeInit\n");
        //log->close();
        if(pAudioCodecCtx)
            avcodec_close(pAudioCodecCtx);

        return Init();
    }

    //视频滤镜初始化
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init] VideoFilterInit\n");
    //log->close();
    if(!(initFlag & 0x20) && VideoFilterInit() < 0)
    {
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("[Init Error] VideoFilterInit\n");
        //log->close();
        if(pVideoBuffersrcCtx)
        {
            avfilter_free(pVideoBuffersrcCtx[0]);
            avfilter_free(pVideoBuffersrcCtx[1]);
        }
        if(pVideoBuffersinkCtx)
            avfilter_free(pVideoBuffersinkCtx);

        return Init();
    }

    //音频滤镜初始化
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init] AudioFilterInit\n");
    //log->close();
    if(!(initFlag & 0x40) && AudioFilterInit() < 0)
    {
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("[Init Error] AudioFilterInit\n");
        //log->close();
        if(pAudioBuffersrcCtx)
            avfilter_free(pAudioBuffersrcCtx);
        if(pAudioBuffersinkCtx)
            avfilter_free(pAudioBuffersinkCtx);

        return Init();
    }

    //输出初始化
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init] OutPutInit\n");
    //log->close();
    if(!(initFlag & 0x80) && OutPutInit() < 0)
    {
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("[Init Error] OutPutInit\n");
        //log->close();
        if (pOutFormatCtx)
        {
            if (!(pOutFormatCtx->oformat->flags & AVFMT_NOFILE))
                avio_close(pOutFormatCtx->pb);
            avformat_free_context(pOutFormatCtx);
        }

        return Init();
    }
}

QString Capture::CaptureStop()
{
    initFlag = 0;
    //log->close();
    isRunning = false;
    this->wait();
    return fileName;
}

int Capture::DesktopCaptureInit()
{
    int ret = 0;
    //参数设置
    AVDictionary* options = NULL;
    av_dict_set_int(&options, "framerate", GlobalData::m_pConfigInfo->frameRate, AV_DICT_MATCH_CASE); //帧率
    av_dict_set_int(&options, "offset_x", GlobalData::m_pConfigInfo->desktopRange_x, AV_DICT_MATCH_CASE); //录制区域的起始坐标-x
    av_dict_set_int(&options, "offset_y", GlobalData::m_pConfigInfo->desktopRange_y, AV_DICT_MATCH_CASE); //录制区域的起始坐标-y
    av_dict_set(&options, "video_size", QString("%1x%2").arg(GlobalData::m_pConfigInfo->desktopWidth).arg(GlobalData::m_pConfigInfo->desktopHeight).toLatin1(), AV_DICT_MATCH_CASE); //录制区域大小
    av_dict_set_int(&options, "draw_mouse", 1, AV_DICT_MATCH_CASE); //是否绘制鼠标

    //获取gdigrab
    AVInputFormat* ifmt = av_find_input_format("gdigrab");

    //打开gidgrab,传入参数,获取封装格式相关信息
    flag = 1;
    while(isRunning)
    {
        pDesktopInFormatCtx = avformat_alloc_context();
        pDesktopInFormatCtx->interrupt_callback.callback = interrupt_callback;
        pDesktopInFormatCtx->interrupt_callback.opaque = &input_runner;
        input_runner.lasttime = time(NULL);

        ret = avformat_open_input(&pDesktopInFormatCtx, "desktop", ifmt, &options);
        qDebug() << "try to open Desktop input time " << flag;
        flag++;
        if(ret >= 0)
        {
            break;
        }
    }
    if(ret < 0)
    {
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Couldn't open input stream.\n");
        //log->close();
        qDebug() << "Couldn't open input stream.";
        return ret;
    }

    //获取流信息
    ret = avformat_find_stream_info(pDesktopInFormatCtx, NULL);
    if(ret < 0 )
    {
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Couldn't find stream information.\n");
        //log->close();
        qDebug() << "Couldn't find stream information.";
        return ret;
    }

    //获取流索引
    for(int i = 0; i < pDesktopInFormatCtx->nb_streams; i++)
    {
        if(pDesktopInFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoIndex = i;
            break;
        }
    }
    if(videoIndex == -1)
    {
        qDebug() << "Didn't find a video stream.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Didn't find a video stream.\n");
        //log->close();
        return ret;
    }
    //av_dump_format(pInFormatCtx, 0, "desktop", 0);

    //为gidgrab创建对应的解码器
    pDesktopCodec = avcodec_find_decoder(pDesktopInFormatCtx->streams[videoIndex]->codecpar->codec_id);
    if(pDesktopCodec == NULL)
    {
        qDebug() << "Codec not found.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Codec not found.\n");
        //log->close();
        return -1;
    }

    pDesktopCodecCtx = avcodec_alloc_context3(pDesktopCodec);
    if (!pDesktopCodecCtx)
    {
        qDebug() << "Can't alloc codec context.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Can't alloc codec context.\n");
        //log->close();
        return -1;
    }

    //复制解码器信息
    ret = avcodec_parameters_to_context(pDesktopCodecCtx, pDesktopInFormatCtx->streams[videoIndex]->codecpar);
    if (ret < 0)
    {
        qDebug() << "Failed to copy context from input to output stream codec context.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Failed to copy context from input to output stream codec context.\n");
        //log->close();
        return ret;
    }
    pDesktopCodecCtx->codec_tag = 0;

    //打开解码器
    ret = avcodec_open2(pDesktopCodecCtx, pDesktopCodec, NULL);
    if(ret < 0)
    {
        qDebug() << "Could not open codec.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Could not open codec.\n");
        //log->close();
        return ret;
    }

    av_dict_free(&options);
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init End] MICCaptureInit\n");
    //log->close();
    initFlag |= 0x01;
    return ret;
}

int Capture::CameraCaptureInit()
{
    int ret = 0;
    //创建AVformatContext句柄
    pCameraInFormatCtx = avformat_alloc_context();

    //获取vfwcap设备
    //AVInputFormat* ifmt = av_find_input_format("vfwcap");
    //ret = avformat_open_input(&pCameraInFormatCtx, "0", ifmt, &options);
    //获取dshow设备
    AVInputFormat* ifmt = av_find_input_format("dshow");
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();

    //打开输入设备
    QString urlString;
    if(GlobalData::m_pConfigInfo->cameraName == "")
    {
        if(GlobalData::m_pConfigInfo->cameraIndex >= 0)
            urlString = QString("video=") + cameras.at(GlobalData::m_pConfigInfo->cameraIndex).description();
        else urlString = QString("video=") + cameras.at(0).description();
    }
    else urlString = QString("video=%1").arg(GlobalData::m_pConfigInfo->cameraName);

    flag = 1;
    while(isRunning)
    {
        pCameraInFormatCtx = avformat_alloc_context();
        pCameraInFormatCtx->interrupt_callback.callback = interrupt_callback;
        pCameraInFormatCtx->interrupt_callback.opaque = &input_runner;
        input_runner.lasttime = time(NULL);

        ret = avformat_open_input(&pCameraInFormatCtx, urlString.toStdString().c_str(), ifmt, NULL);
        qDebug() << "try to open Camera input time " << flag;

        flag++;
        if(ret >= 0)
        {
            break;
        }
    }
    if(ret < 0)
    {
        qDebug() << "Couldn't open camera.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Could not open camera.\n");
        //log->close();
        return ret;
    }

    //获取流信息
    ret = avformat_find_stream_info(pCameraInFormatCtx, NULL);
    if(ret < 0 )
    {
        qDebug() << "Couldn't find stream information.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Couldn't find stream information.\n");
        //log->close();
        return ret;
    }

    //获取流索引
    for(int i = 0; i < pCameraInFormatCtx->nb_streams; i++)
    {
        if(pCameraInFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoIndexC = i;
            break;
        }
    }
    if(videoIndexC == -1)
    {
        qDebug() << "Didn't find a video stream.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Didn't find a video stream.\n");
        //log->close();
        return ret;
    }

    //创建对应的解码器
    pCameraCodec = avcodec_find_decoder(pCameraInFormatCtx->streams[videoIndexC]->codecpar->codec_id);
    if(pCameraCodec == NULL)
    {
        qDebug() << "Codec not found.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Codec not found.\n");
        //log->close();
        return -1;
    }

    pCameraCodecCtx = avcodec_alloc_context3(pCameraCodec);
    if (!pCameraCodecCtx)
    {
        qDebug() << "Can't alloc codec context.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Can't alloc codec context.\n");
        //log->close();
        return -1;
    }

    //复制解码器信息
    ret = avcodec_parameters_to_context(pCameraCodecCtx, pCameraInFormatCtx->streams[videoIndexC]->codecpar);
    if (ret < 0)
    {
        qDebug() << "Failed to copy context from input to output stream codec context.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Failed to copy context from input to output stream codec context.\n");
        //log->close();
        return ret;
    }
    pCameraCodecCtx->codec_tag = 0;

    //打开解码器
    AVDictionary* options = NULL;
    av_dict_set(&options, "rtbufsize", "800k", 0);
    ret = avcodec_open2(pCameraCodecCtx, pCameraCodec, &options);
    if(ret < 0)
    {
        qDebug() << "Could not open codec.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Could not open codec.\n");
        //log->close();
        return ret;
    }
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init End] CameraCaptureInit\n");
    //log->close();
    initFlag |= 0x02;
    return ret;
}

int Capture::MICCaptureInit()
{
    int ret = 0;
    //创建AVformatContext句柄
    pMICInFormatCtx = avformat_alloc_context();

    //获取dshow设备
    AVInputFormat* ifmt = av_find_input_format("dshow");
    QList<QAudioDeviceInfo> audioDevice = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    //打开输入设备
    AVDictionary* opts = nullptr;
    av_dict_set_int(&opts, "audio_buffer_size", 30, 0);

    QString urlString;
    if(GlobalData::m_pConfigInfo->MICName == "")
    {
        if(GlobalData::m_pConfigInfo->MICIndex >= 0)
            urlString = QString("audio=") + audioDevice.at(GlobalData::m_pConfigInfo->MICIndex).deviceName();
        else urlString = QString("audio=") + audioDevice.at(0).deviceName();
    }
    else urlString = QString("audio=%1").arg(GlobalData::m_pConfigInfo->MICName);

    flag = 1;
    while(isRunning)
    {
        pMICInFormatCtx = avformat_alloc_context();
        pMICInFormatCtx->interrupt_callback.callback = interrupt_callback;
        pMICInFormatCtx->interrupt_callback.opaque = &input_runner;
        input_runner.lasttime = time(NULL);

        ret = avformat_open_input(&pMICInFormatCtx, urlString.toStdString().c_str(), ifmt, &opts);
        qDebug() << "try to open MIC input time " << flag;
        flag++;
        if(ret >= 0)
        {
            break;
        }
    }
    if(ret < 0)
    {
        qDebug() << "Couldn't open MIC.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Couldn't open MIC.\n");
        //log->close();
        return ret;
    }

    //获取流信息
    ret = avformat_find_stream_info(pMICInFormatCtx, NULL);
    if(ret < 0 )
    {
        qDebug() << "Couldn't find stream information.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Couldn't find stream information.\n");
        //log->close();
        return ret;
    }

    //获取流索引
    for(int i = 0; i < pMICInFormatCtx->nb_streams; i++)
    {
        if(pMICInFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioIndex = i;
            break;
        }
    }
    if(audioIndex == -1)
    {
        qDebug() << "Didn't find a video stream.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Didn't find a video stream.\n");
        //log->close();
        return ret;
    }

    //创建对应的解码器
    pMICCodec = avcodec_find_decoder(pMICInFormatCtx->streams[audioIndex]->codecpar->codec_id);
    if(pMICCodec == NULL)
    {
        qDebug() << "Codec not found.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Codec not found.\n");
        //log->close();
        return -1;
    }

    pMICCodecCtx = avcodec_alloc_context3(pMICCodec);
    if (!pMICCodecCtx)
    {
        qDebug() << "Can't alloc codec context.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Can't alloc codec context.\n");
        //log->close();
        return -1;
    }

    //复制解码器信息
    ret = avcodec_parameters_to_context(pMICCodecCtx, pMICInFormatCtx->streams[audioIndex]->codecpar);
    if (ret < 0)
    {
        qDebug() << "Failed to copy context from input to output stream codec context.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Failed to copy context from input to output stream codec context.\n");
        //log->close();
        return ret;
    }

    pMICCodecCtx->channel_layout = av_get_default_channel_layout(pMICCodecCtx->channels);
    pMICCodecCtx->codec_tag = 0;

    //打开解码器
    ret = avcodec_open2(pMICCodecCtx, pMICCodec, NULL);
    if(ret < 0)
    {
        qDebug() << "Could not open codec.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Could not open codec.\n");
        //log->close();
        return ret;
    }
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init End] MICCaptureInit\n");
    //log->close();
    initFlag |= 0x04;
    return ret;
}

int Capture::H264RecodeInit()
{
    int ret = 0;
    AVDictionary* options = NULL;
    //查找H264编码器
    pH264Codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!pH264Codec)
    {
        qDebug() << "Could not find h264 codec.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Could not find h264 codec.\n");
        //log->close();
        return -1;
    }

    // 设置参数
    pH264CodecCtx = avcodec_alloc_context3(pH264Codec);
    pH264CodecCtx->codec_id = AV_CODEC_ID_H264;
    pH264CodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pH264CodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    pH264CodecCtx->width = GlobalData::m_pConfigInfo->desktopWidth;
    pH264CodecCtx->height = GlobalData::m_pConfigInfo->desktopHeight;
    pH264CodecCtx->time_base.num = 1;
    pH264CodecCtx->time_base.den = GlobalData::m_pConfigInfo->frameRate;	//帧率（即一秒钟多少张图片）
    pH264CodecCtx->bit_rate = GlobalData::m_pConfigInfo->videoBitRate;	//比特率（调节这个大小可以改变编码后视频的质量）
    pH264CodecCtx->rc_max_rate = GlobalData::m_pConfigInfo->videoBitRate;
    pH264CodecCtx->rc_min_rate = GlobalData::m_pConfigInfo->videoBitRate;
    //pH264CodecCtx->rc_buffer_size = GlobalData::m_pConfigInfo->videoBitRate / 2;
    pH264CodecCtx->gop_size = 2;
    pH264CodecCtx->qmin = 10;
    pH264CodecCtx->qmax = 51;
    pH264CodecCtx->max_b_frames = 0;
    pH264CodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER | AV_CODEC_FLAG_LOW_DELAY;

    //打开H.264编码器
    av_dict_set(&options, "preset", "superfast", 0);
    av_dict_set(&options, "tune", "zerolatency", 0);//实现实时编码
    av_dict_set(&options, "rtbufsize", "800k", 0);
    av_dict_set(&options, "start_time_realtime", 0, 0);
    av_dict_set(&options, "crf", "18", 0);
    av_dict_set(&options, "level", "4",0);

    ret = avcodec_open2(pH264CodecCtx, pH264Codec, &options);
    if ( ret< 0)
    {
        qDebug() << "Could not open video encoder.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Could not open video encoder.\n");
        //log->close();
        return ret;
    }
    av_dict_free(&options);
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init End] H264RecodeInit\n");
    //log->close();
    initFlag |= 0x08;
    return ret;
}

int Capture::AudioRecodeInit()
{
    int ret = 0;
    AVDictionary* options = NULL;
    //查找音频编码器
    pAudioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!pAudioCodec)
    {
        qDebug() << "Could not find h264 codec.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Could not find h264 codec.\n");
        //log->close();
        return -1;
    }

    //设置参数
    pAudioCodecCtx = avcodec_alloc_context3(pAudioCodec);
    pAudioCodecCtx->codec = pAudioCodec;
    pAudioCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    pAudioCodecCtx->sample_rate= GlobalData::m_pConfigInfo->sampleRate; //音频采样率
    pAudioCodecCtx->channels = 2;
    pAudioCodecCtx->channel_layout = av_get_default_channel_layout(pAudioCodecCtx->channels);
    pAudioCodecCtx->bit_rate = GlobalData::m_pConfigInfo->audioBitRate;
    pAudioCodecCtx->codec_tag = 0;
    pAudioCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    av_dict_set(&options, "preset", "superfast", 0);
    av_dict_set(&options, "tune", "zerolatency", 0);//实现实时编码
    av_dict_set(&options, "rtbufsize", "500k", 0);
    av_dict_set(&options, "start_time_realtime", 0, 0);

    //打开音频编码器
    ret = avcodec_open2(pAudioCodecCtx, pAudioCodec, &options);
    if ( ret< 0)
    {
        qDebug() << "Could not open audio encoder.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Could not open audio encoder.\n");
        //log->close();
        return ret;
    }
    av_dict_free(&options);
    initFlag |= 0x10;
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init End] AudioRecodeInit\n");
    //log->close();
    return ret;
}

int Capture::VideoFilterInit()
{
    char args[2][512] ={ { 0 }, { 0 } };
    int ret = 0;
    AVFilter** buffersrc = (AVFilter**)av_malloc(4 * sizeof(AVFilter*));
    buffersrc[0] = (AVFilter*)avfilter_get_by_name("buffer");
    buffersrc[1] = (AVFilter*)avfilter_get_by_name("buffer");
    AVFilter* buffersink = (AVFilter*)avfilter_get_by_name("buffersink");

    pVideoBuffersrcCtx = (AVFilterContext**)av_malloc(2 * sizeof(AVFilterContext*));
    pVideoBuffersrcCtx[0] = NULL;
    pVideoBuffersrcCtx[1] = NULL;


    //buffersrc的输出引脚
    AVFilterInOut** outputs = (AVFilterInOut**)av_malloc(2 * sizeof(AVFilterInOut*));
    outputs[0] = avfilter_inout_alloc(); //桌面视频流
    outputs[1] = avfilter_inout_alloc(); //摄像头视频流
    AVFilterInOut* input  = avfilter_inout_alloc(); //输出视频流

    AVFilterGraph* filter_graph = avfilter_graph_alloc();
    AVBufferSinkParams *buffersink_params = av_buffersink_params_alloc();
    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };

    QString filter_descr = QString("[in0]pad=%1:%2[x1];[in1]scale=w=%3:h=%4[inn1];[x1][inn1]overlay=%5:%6[out]")
            .arg(GlobalData::m_pConfigInfo->desktopWidth).arg(GlobalData::m_pConfigInfo->desktopHeight)
            .arg(GlobalData::m_pConfigInfo->cameraWidth).arg(GlobalData::m_pConfigInfo->cameraHeight)
            .arg(GlobalData::m_pConfigInfo->cameraPos_x).arg(GlobalData::m_pConfigInfo->cameraPos_y);

    //创建in - 桌面视频流
    snprintf(args[0], sizeof(args[0]), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                        GlobalData::m_pConfigInfo->desktopWidth, GlobalData::m_pConfigInfo->desktopHeight, AV_PIX_FMT_YUV420P, 1, GlobalData::m_pConfigInfo->frameRate, 0, 1);

    ret = avfilter_graph_create_filter(&pVideoBuffersrcCtx[0], buffersrc[0], "in0", args[0], NULL, filter_graph);
    if (ret < 0)
    {
        qDebug() << "Cannot create buffer source";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Cannot create buffer source.\n");
        //log->close();
        return ret;
    }

    //创建in - 摄像头视频流
    //snprintf(args[1], sizeof(args[1]), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
    //        GlobalData::m_pConfigInfo->cameraWidth, GlobalData::m_pConfigInfo->cameraHeight, AV_PIX_FMT_YUV420P, 1, GlobalData::m_pConfigInfo->frameRate, 0, 1);
    snprintf(args[1], sizeof(args[1]), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
            pCameraCodecCtx->width, pCameraCodecCtx->height, AV_PIX_FMT_YUV420P, 1, GlobalData::m_pConfigInfo->frameRate, 0, 1);

    ret = avfilter_graph_create_filter(&pVideoBuffersrcCtx[1], buffersrc[1], "in1", args[1], NULL, filter_graph);
    if (ret < 0)
    {
        qDebug() << "Cannot create buffer source.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Cannot create buffer source.\n");
        //log->close();
        return ret;
    }

    //创建 out
    buffersink_params->pixel_fmts = pix_fmts;
    ret = avfilter_graph_create_filter(&pVideoBuffersinkCtx, buffersink, "out", NULL, buffersink_params, filter_graph);
    av_free(buffersink_params);
    if (ret < 0)
    {
        qDebug() << "Cannot create buffer sink.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Cannot create buffer sink.\n");
        //log->close();
        return ret;
    }

    outputs[0]->name = av_strdup("in0");
    outputs[0]->filter_ctx = pVideoBuffersrcCtx[0];
    outputs[0]->pad_idx = 0;
    outputs[0]->next = outputs[1];

    outputs[1]->name = av_strdup("in1");
    outputs[1]->filter_ctx = pVideoBuffersrcCtx[1];
    outputs[1]->pad_idx = 0;
    outputs[1]->next = NULL;

    //sink的输入引脚，连接到 滤镜图 里面的 out标签
    input->name = av_strdup("out");
    input->filter_ctx = pVideoBuffersinkCtx;
    input->pad_idx = 0;
    input->next = NULL;

    ret = avfilter_graph_parse_ptr(filter_graph, filter_descr.toLatin1(), &input, outputs, NULL);
    if (ret < 0)
    {
        qDebug() << "Add video graph described failed.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Add video graph described failed.\n");
        //log->close();
        return ret;
    }

    ret = avfilter_graph_config(filter_graph, NULL);
    if (ret < 0)
    {
        qDebug() << "Video filter init failed.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Video filter init failed.\n");
        //log->close();
        return ret;
    }

    avfilter_inout_free(&input);
    avfilter_inout_free(outputs);
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init End] VideoFilterInit\n");
    //log->close();
    initFlag |= 0x20;
    return 0;
}

int Capture::AudioFilterInit()
{
    char args[512];
    int ret;
    AVFilter* abuffersrc = (AVFilter*)avfilter_get_by_name("abuffer");
    AVFilter* abuffersink = (AVFilter*)avfilter_get_by_name("abuffersink");
    AVFilterInOut* outputs = avfilter_inout_alloc();
    AVFilterInOut* inputs = avfilter_inout_alloc();

    AVFilterGraph* filterGraph = avfilter_graph_alloc();
    filterGraph->nb_threads = 1;

    static const enum AVSampleFormat out_sample_fmts[] = { AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_NONE };
    static const int64_t out_channel_layouts[] = { (uint8_t)pAudioCodecCtx->channel_layout, -1 };
    static const int out_sample_rates[] = { pAudioCodecCtx->sample_rate , -1 };

    AVRational time_base = pMICInFormatCtx->streams[audioIndex]->time_base;
    sprintf_s(args, sizeof(args),
        "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%I64x",
        time_base.num, time_base.den, pMICCodecCtx->sample_rate,
        av_get_sample_fmt_name(pMICCodecCtx->sample_fmt), pMICCodecCtx->channel_layout);

    ret = avfilter_graph_create_filter(&pAudioBuffersrcCtx, abuffersrc, "in", args, NULL, filterGraph);
    if (ret < 0)
    {
        qDebug() << "Cannot create audio buffer source.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Cannot create audio buffer source.\n");
        //log->close();
        return ret;
    }

    /* buffer audio sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&pAudioBuffersinkCtx, abuffersink, "out", NULL, NULL, filterGraph);
    if (ret < 0)
    {
        qDebug() << "Cannot create audio buffer sink.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Cannot create audio buffer sink.\n");
        //log->close();
        return ret;
    }

    ret = av_opt_set_int_list(pAudioBuffersinkCtx, "sample_fmts", out_sample_fmts, -1, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        qDebug() << "Cannot set output sample format.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Cannot set output sample format.\n");
        //log->close();
        return ret;
    }

    ret = av_opt_set_int_list(pAudioBuffersinkCtx, "channel_layouts", out_channel_layouts, -1, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        qDebug() << "Cannot set output channel layout.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Cannot set output channel layout.\n");
        //log->close();
        return ret;
    }

    ret = av_opt_set_int_list(pAudioBuffersinkCtx, "sample_rates", out_sample_rates, -1, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        qDebug() << "Cannot set output sample rate.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Cannot set output sample rate.\n");
        //log->close();
        return ret;
    }

    /* Endpoints for the filter graph. */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = pAudioBuffersrcCtx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = pAudioBuffersinkCtx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    ret = avfilter_graph_parse_ptr(filterGraph, "anull", &inputs, &outputs, nullptr);
    if (ret < 0)
    {
        qDebug() << "Add audio graph described failed.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Add audio graph described failed.\n");
        //log->close();
        return ret;
    }

    ret = avfilter_graph_config(filterGraph, NULL);
    if (ret < 0)
    {
        qDebug() << "Audio filter init failed.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Audio filter init failed.\n");
        //log->close();
        return ret;
    }

//    av_buffersink_set_frame_size(pAudioBuffersinkCtx, 1024);
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init End] AudioFilterInit\n");
    //log->close();
    initFlag |= 0x40;
    return 0;
}

int Capture::OutPutInit()
{
    int ret = 0;
    AVStream* outVideoStream = NULL;
    AVStream* outAudioStream = NULL;
    AVDictionary * options = NULL;
    av_dict_set(&options, "flvflags", "no_duration_filesize", 0);
    //分配输出ctx
#ifdef MODE_RECORD
    fileName = QString("Record_%1.flv")
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));
    avformat_alloc_output_context2(&pOutFormatCtx, NULL, "flv", fileName.toLatin1());
#elif defined MODE_CAPTURE
    avformat_alloc_output_context2(&pOutFormatCtx, NULL, "flv", GlobalData::m_pConfigInfo->outUrl.toLatin1());
#endif
    if (!pOutFormatCtx)
    {
        qDebug() << "Could not create output context.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Could not create output context.\n");
        //log->close();
        return -1;
    }

    //创建输出视频流
    outVideoStream = avformat_new_stream(pOutFormatCtx, pH264Codec);
    if (!outVideoStream)
    {
        qDebug() << "Failed allocating output stream.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Failed allocating output stream.\n");
        //log->close();
        return -1;
    }
    avcodec_parameters_from_context(outVideoStream->codecpar, pH264CodecCtx);
    outVideoStream->codecpar->codec_tag = 0;
    outVideoIndex = outVideoStream->index;

    //创建输出音频流
    outAudioStream = avformat_new_stream(pOutFormatCtx, pAudioCodec);
    if (!outAudioStream)
    {
        qDebug() << "Failed allocating output stream.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Failed allocating output stream.\n");
        //log->close();
        return -1;
    }
    //avcodec_copy_context(outAudioStream->codec, pAudioCodecCtx);
    avcodec_parameters_from_context(outAudioStream->codecpar, pAudioCodecCtx);
    outAudioStream->codecpar->codec_tag = 0;
    outAudioIndex = outAudioStream->index;

    //打开输出url
    if (!(pOutFormatCtx->oformat->flags & AVFMT_NOFILE))
    {
#ifdef MODE_RECORD
        ret = avio_open(&pOutFormatCtx->pb, fileName.toLatin1(), AVIO_FLAG_WRITE);
#elif defined MODE_CAPTURE
        ret = avio_open(&pOutFormatCtx->pb, GlobalData::m_pConfigInfo->outUrl.toLatin1(), AVIO_FLAG_WRITE);
#endif
        if (ret < 0)
        {
            qDebug() << "Failed to open output url: " << GlobalData::m_pConfigInfo->outUrl << ". Error Code : " << ret;
            //log->open(QIODevice::WriteOnly | QIODevice::Append);
            //log->write("Failed to open output url.\n");
            //log->close();
            return ret;
        }
    }

    //写文件头
    ret = avformat_write_header(pOutFormatCtx, options ? &options : NULL);
    if (ret < 0)
    {
        qDebug() << "Failed to write header. Error Code : " << ret;
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Failed to write header.\n");
        //log->close();
        return ret;
    }
    av_dict_free(&options);
    initFlag |= 0x80;
    //log->open(QIODevice::WriteOnly | QIODevice::Append);
    //log->write("[Init End] OutPutInit\n");
    //log->close();
    return ret;
}

int Capture::DesktoCaptureStart()
{
    int ret = 0;
    int frameIndex = 0;

    unsigned char* outBuffer = NULL;

    //写文件头
    SwsContext* pImgConvertCtx = NULL;
    AVPacket* packet = av_packet_alloc();
    AVFrame* pFrame = av_frame_alloc();
    AVFrame* pFrameYUV = av_frame_alloc();


    outBuffer = (unsigned char*)av_malloc(av_image_get_buffer_size(
                              AV_PIX_FMT_YUV420P, pDesktopCodecCtx->width, pDesktopCodecCtx->height, 1));

    av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, outBuffer,
            AV_PIX_FMT_YUV420P, pDesktopCodecCtx->width, pDesktopCodecCtx->height, 1);

    pImgConvertCtx = sws_getContext(pDesktopCodecCtx->width, pDesktopCodecCtx->height,
            pDesktopCodecCtx->pix_fmt, pDesktopCodecCtx->width, pDesktopCodecCtx->height,
            AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);


    while (isRunning)
    {
        //从输入流读取一个packet
        ret = av_read_frame(pDesktopInFormatCtx, packet);
        if (ret < 0)
        {
            qDebug() << "Read frame failed.";
            break;
        }


        if (packet->stream_index == videoIndex)
        {
            ret = avcodec_send_packet(pDesktopCodecCtx, packet);
            if (ret < 0)
            {
                qDebug() << "Decode packet failed.";
                break;
            }
            pFrame->format = pDesktopCodecCtx->sample_fmt;
            pFrame->width = pDesktopCodecCtx->width;
            pFrame->height = pDesktopCodecCtx->height;

            ret = avcodec_receive_frame(pDesktopCodecCtx, pFrame);
            if (ret >= 0)
            {
                sws_scale(pImgConvertCtx,(const unsigned char* const*)pFrame->data,
                    pFrame->linesize, 0, pDesktopCodecCtx->height, pFrameYUV->data,pFrameYUV->linesize);

                pFrameYUV->format = pDesktopCodecCtx->pix_fmt;
                pFrameYUV->width = pDesktopCodecCtx->width;
                pFrameYUV->height = pDesktopCodecCtx->height;

                ret = avcodec_send_frame(pH264CodecCtx, pFrameYUV);
                if (ret < 0)
                {
                    qDebug() << "Failed to encode frame.";
                    break;
                }

                while (avcodec_receive_packet(pH264CodecCtx, packet) >= 0)
                {
                    // 设置输出DTS,PTS
                    packet->pts = packet->dts = frameIndex * (pOutFormatCtx->streams[0]->time_base.den) / pOutFormatCtx->streams[0]->time_base.num / GlobalData::m_pConfigInfo->frameRate;
                    frameIndex++;

                    ret = av_interleaved_write_frame(pOutFormatCtx, packet);
                    if (ret < 0)
                    {
                        qDebug() << "Failed to write frame to url";
                        break;
                    }
                    else qDebug() << "Send" << frameIndex << " packet to url successfully";
                }

            }
        }
        av_packet_unref(packet);
    }

    //写文件尾
    av_write_trailer(pOutFormatCtx);
    av_packet_free(&packet);
    av_frame_free(&pFrame);
    av_frame_free(&pFrameYUV);
    if(ret < 0)
        emit Error();
    return ret;
}

int Capture::MergeCaptureStart()
{
    int ret = 0;
    AVFrame* pFrameFilt = av_frame_alloc();

    //桌面视频流Frame初始化
    SwsContext* pDesktopImgConvertCtx = NULL;
    AVPacket* pDesktopPkt = av_packet_alloc();
    AVFrame* pDesktopFrame = av_frame_alloc();   
    AVFrame* pDesktopFrameYUV = av_frame_alloc();


    unsigned char* DesktopBuffer = (unsigned char*)av_malloc(av_image_get_buffer_size(
                              AV_PIX_FMT_YUV420P, pDesktopCodecCtx->width, pDesktopCodecCtx->height, 1));

    av_image_fill_arrays(pDesktopFrameYUV->data, pDesktopFrameYUV->linesize, DesktopBuffer,
            AV_PIX_FMT_YUV420P, pDesktopCodecCtx->width, pDesktopCodecCtx->height, 1);

    pDesktopImgConvertCtx = sws_getContext(pDesktopCodecCtx->width, pDesktopCodecCtx->height,
            pDesktopCodecCtx->pix_fmt, pDesktopCodecCtx->width, pDesktopCodecCtx->height,
            AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    //摄像头视频流Frame初始化
    SwsContext* pCameraImgConvertCtx = NULL;
    AVPacket* pCameraPkt = av_packet_alloc();
    AVFrame* pCameraFrame = av_frame_alloc();
    AVFrame* pCameraFrameYUV = av_frame_alloc();

    unsigned char* CameraBuffer = (unsigned char*)av_malloc(av_image_get_buffer_size(
                              AV_PIX_FMT_YUV420P, pCameraCodecCtx->width, pCameraCodecCtx->height, 1));

    av_image_fill_arrays(pCameraFrameYUV->data, pCameraFrameYUV->linesize, CameraBuffer,
            AV_PIX_FMT_YUV420P, pCameraCodecCtx->width, pCameraCodecCtx->height, 1);

    pCameraImgConvertCtx = sws_getContext(pCameraCodecCtx->width, pCameraCodecCtx->height,
            pCameraCodecCtx->pix_fmt, pCameraCodecCtx->width, pCameraCodecCtx->height,
            AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);


    //申请缓存
    int size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pDesktopCodecCtx->width, pDesktopCodecCtx->height, 1);
    pVideoFifo = av_fifo_alloc(30 * size);

    while (isRunning)
    {
        if(!pDesktopCodecCtx || !pCameraCodecCtx)
        {
            emit Error();
            return ret;
        }

        av_image_fill_arrays(pDesktopFrameYUV->data, pDesktopFrameYUV->linesize, DesktopBuffer,
                AV_PIX_FMT_YUV420P, pDesktopCodecCtx->width, pDesktopCodecCtx->height, 1);
        av_image_fill_arrays(pCameraFrameYUV->data, pCameraFrameYUV->linesize, CameraBuffer,
                AV_PIX_FMT_YUV420P, pCameraCodecCtx->width, pCameraCodecCtx->height, 1);
        //msleep(10);
        pDesktopFrame->format = pDesktopCodecCtx->pix_fmt;
        pDesktopFrame->width = pDesktopCodecCtx->width;
        pDesktopFrame->height = pDesktopCodecCtx->height;

        ret = GetDesktopFrame(pDesktopPkt, pDesktopFrame, pDesktopFrameYUV, pDesktopImgConvertCtx);
        if (ret < 0)
        {
            qDebug() << "Get desktop frame failed.";
            //log->open(QIODevice::WriteOnly | QIODevice::Append);
            //log->write("Get desktop frame failed.\n");
            //log->close();
            break;
        }
        pCameraFrame->format = pCameraCodecCtx->sample_fmt;
        pCameraFrame->width = pCameraCodecCtx->width;
        pCameraFrame->height = pCameraCodecCtx->height;

        ret = GetCameraFrame(pCameraPkt, pCameraFrame, pCameraFrameYUV, pCameraImgConvertCtx);
        if (ret < 0)
        {
            qDebug() << "Get camera frame failed.";
            //log->open(QIODevice::WriteOnly | QIODevice::Append);
            //log->write("Get camera frame failed.\n");
            //log->close();
            break;
        }

        if(!pH264CodecCtx || !pVideoBuffersinkCtx)
        {
            emit Error();
            return ret;
        }
        pFrameFilt->format = pH264CodecCtx->pix_fmt;
        pFrameFilt->width = pH264CodecCtx->width;
        pFrameFilt->height = pH264CodecCtx->height;

        ret = av_buffersink_get_frame(pVideoBuffersinkCtx, pFrameFilt);
        if (ret < 0)
        {
            qDebug() << "Get Filt frame failed.";
            //log->open(QIODevice::WriteOnly | QIODevice::Append);
            //log->write("Get Filt frame failed.\n");
            //log->close();
            continue;
        }
        //qDebug() << "Get video frame.\n";

        if (pVideoFifo && av_fifo_space(pVideoFifo) > size)
        {
            av_fifo_generic_write(pVideoFifo, pFrameFilt->data[0], pFrameFilt->linesize[0] * pH264CodecCtx->height, NULL);
            av_fifo_generic_write(pVideoFifo, pFrameFilt->data[1], pFrameFilt->linesize[1] * pH264CodecCtx->height / 2, NULL);
            av_fifo_generic_write(pVideoFifo, pFrameFilt->data[2], pFrameFilt->linesize[2] * pH264CodecCtx->height / 2, NULL);
            //qDebug() << "fifo video frame.";
        }
        av_packet_unref(pDesktopPkt);
        av_packet_unref(pCameraPkt);

        av_frame_unref(pDesktopFrame);
        av_frame_unref(pCameraFrame);
        av_frame_unref(pFrameFilt);
        av_frame_unref(pDesktopFrame);
        av_frame_unref(pDesktopFrameYUV);
    }

    av_frame_free(&pDesktopFrame);
    av_frame_free(&pDesktopFrameYUV);
    av_frame_free(&pCameraFrame);
    av_frame_free(&pCameraFrameYUV);
    av_frame_free(&pFrameFilt);

    if(ret < 0)
        emit Error();
    return ret;
}

int Capture::MICCaptureStart()
{
    int ret = 0;

    AVPacket* pMicPkt = av_packet_alloc();
    AVFrame* pMicFrame = av_frame_alloc();
    AVFrame* pFrameFilt = av_frame_alloc();

    pAudioFifo = av_audio_fifo_alloc(pAudioCodecCtx->sample_fmt,
        pAudioCodecCtx->channels, 30 * 1024);

    while (isRunning)
    {
        //从输入流读取一个packet
        input_runner.lasttime = time(NULL);
        if(!pMICInFormatCtx)
        {
            emit Error();
            return ret;
        }

        ret = av_read_frame(pMICInFormatCtx, pMicPkt);
        if (ret < 0)
        {
            qDebug() << "Read audio frame failed.";
            //log->open(QIODevice::WriteOnly | QIODevice::Append);
            //log->write("Read audio frame failed.\n");
            //log->close();
            break;
        }

        if (pMicPkt->stream_index == audioIndex)
        {
            if(!pMICCodecCtx)
            {
                emit Error();
                return ret;
            }
            ret = avcodec_send_packet(pMICCodecCtx, pMicPkt);
            if (ret < 0)
            {
                qDebug() << "Decode audio frame failed.";
                //log->open(QIODevice::WriteOnly | QIODevice::Append);
                //log->write("Decode audio frame failed.\n");
                //log->close();
                break;
            }
            pMicFrame->format = pMICCodecCtx->sample_fmt;
            while(avcodec_receive_frame(pMICCodecCtx, pMicFrame) >= 0)
            {
                if(!pAudioBuffersrcCtx) break;
                ret = av_buffersrc_add_frame_flags(pAudioBuffersrcCtx, pMicFrame, AV_BUFFERSRC_FLAG_PUSH);
                if (ret < 0)
                {
                    qDebug() << "Error while feeding the filtergraph.";
                    //log->open(QIODevice::WriteOnly | QIODevice::Append);
                    //log->write("Error while feeding the filtergraph.\n");
                    //log->close();
                    break;
                }

                if(!pAudioCodecCtx || !pAudioBuffersinkCtx)
                {
                    emit Error();
                    return ret;
                }
                pFrameFilt->format = pAudioCodecCtx->sample_fmt;
                ret = av_buffersink_get_frame_flags(pAudioBuffersinkCtx, pFrameFilt, AV_BUFFERSINK_FLAG_NO_REQUEST);
                if (ret < 0)
                {
                    qDebug() << "Error while feeding the filtergraph.";
                    //log->open(QIODevice::WriteOnly | QIODevice::Append);
                    //log->write("Error while feeding the filtergraph.\n");
                    //log->close();
                    break;
                }
                //qDebug() << "Get video frame.";

                if (pAudioFifo && av_audio_fifo_space(pAudioFifo) >= 1024)
                {
                    av_audio_fifo_write(pAudioFifo, (void **)pFrameFilt->data, pFrameFilt->nb_samples);
                    //qDebug() << "fifo audio frame.";
                }
            }
        }
        av_packet_unref(pMicPkt);
        av_frame_unref(pMicFrame);
        av_frame_unref(pFrameFilt);
    }

    av_frame_free(&pMicFrame);
    av_frame_free(&pFrameFilt);
    av_packet_free(&pMicPkt);

    if(ret < 0)
        emit Error();
    return ret;
}

int Capture::GetDesktopFrame(AVPacket* pPacket, AVFrame* pFrame, AVFrame* pFrameYUV, SwsContext* pSwsCtx)
{
    int ret = 0;

    input_runner.lasttime = time(NULL);
    if(!pDesktopInFormatCtx)
        return -1;
    ret = av_read_frame(pDesktopInFormatCtx, pPacket);
    if (ret < 0)
    {
        qDebug() << "Read desktop frame failed.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Read desktop frame failed.\n");
        //log->close();
        return ret;
    }

    if (pPacket->stream_index == videoIndex)
    {
        if(!pDesktopCodecCtx)
            return -1;
        ret = avcodec_send_packet(pDesktopCodecCtx, pPacket);
        if (ret < 0)
        {
            qDebug() << "Decode packet failed.";
            //log->open(QIODevice::WriteOnly | QIODevice::Append);
            //log->write("Decode packet failed.\n");
            //log->close();
            return ret;
        }

        ret = avcodec_receive_frame(pDesktopCodecCtx, pFrame);
        if (ret >= 0)
        {
            sws_scale(pSwsCtx,(const unsigned char* const*)pFrame->data, pFrame->linesize,
                      0, pDesktopCodecCtx->height, pFrameYUV->data,pFrameYUV->linesize);

            pFrameYUV->format = AV_PIX_FMT_YUV420P;
            pFrameYUV->width = pDesktopCodecCtx->width;
            pFrameYUV->height = pDesktopCodecCtx->height;

            if(!pVideoBuffersrcCtx[0])
                return -1;
            ret = av_buffersrc_add_frame_flags(pVideoBuffersrcCtx[0], pFrameYUV, AV_BUFFERSRC_FLAG_KEEP_REF );
            if (ret < 0)
            {
                qDebug() << "Error while feeding the filtergraph.";
                //log->open(QIODevice::WriteOnly | QIODevice::Append);
                //log->write("Error while feeding the filtergraph.\n");
                //log->close();
                return ret;
            }
        }
    }

    return ret;
}

int Capture::GetCameraFrame(AVPacket* pPacket, AVFrame* pFrame, AVFrame* pFrameYUV, SwsContext* pSwsCtx)
{
    int ret = 0;
    if(!pCameraInFormatCtx)
        return -1;
    input_runner.lasttime = time(NULL);
    ret = av_read_frame(pCameraInFormatCtx, pPacket);
    if (ret < 0)
    {
        qDebug() << "Read camera frame failed.";
        //log->open(QIODevice::WriteOnly | QIODevice::Append);
        //log->write("Read camera frame failed.\n");
        //log->close();
        return ret;
    }

    if (pPacket->stream_index == videoIndexC)
    {
        if(!pCameraCodecCtx)
            return -1;
        ret = avcodec_send_packet(pCameraCodecCtx, pPacket);
        if (ret < 0)
        {
            qDebug() << "Decode packet failed.";
            //log->open(QIODevice::WriteOnly | QIODevice::Append);
            //log->write("Decode packet failed.\n");
            //log->close();
            return ret;
        }

        ret = avcodec_receive_frame(pCameraCodecCtx, pFrame);
        if (ret >= 0)
        {

            sws_scale(pSwsCtx,(const unsigned char* const*)pFrame->data, pFrame->linesize,
                      0, pCameraCodecCtx->height, pFrameYUV->data,pFrameYUV->linesize);

            pFrameYUV->format = AV_PIX_FMT_YUV420P;
            pFrameYUV->width = pCameraCodecCtx->width;
            pFrameYUV->height = pCameraCodecCtx->height;

            if(!pVideoBuffersrcCtx[1])
                return -1;
            ret = av_buffersrc_add_frame_flags(pVideoBuffersrcCtx[1], pFrameYUV, AV_BUFFERSRC_FLAG_KEEP_REF);
            if (ret < 0)
            {
                qDebug() << "Error while feeding the filtergraph.";
                //log->open(QIODevice::WriteOnly | QIODevice::Append);
                //log->write("Error while feeding the filtergraph.\n");
                //log->close();
                return ret;
            }
        }
    }
    return ret;
}

int Capture::CaptureStart()
{
    int ret = 0;
    int64_t cur_pts_v=0;
    int64_t cur_pts_a=0;
    int64_t VideoFrameIndex = 0;
    int64_t AudioFrameIndex = 0;
    int ptsInterval = 0;


    AVFrame* pOutFrame = av_frame_alloc();

    AVPacket* pOutPkt = av_packet_alloc();

    //缓冲数据初始化
    uint8_t* frame_buf = NULL;
    int size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, GlobalData::m_pConfigInfo->desktopWidth, GlobalData::m_pConfigInfo->desktopHeight, 1);
    frame_buf = new uint8_t[size];
    memset(frame_buf, 0, size);
    av_image_fill_arrays(pOutFrame->data, pOutFrame->linesize, frame_buf, AV_PIX_FMT_YUV420P,
           GlobalData::m_pConfigInfo->desktopWidth, GlobalData::m_pConfigInfo->desktopHeight, 1);


    isRunning = true;
    while (isRunning)
    {
        //比较pst,判断帧种类
        if(!pOutFormatCtx)
            break;

        msleep(1);
        if(initFlag & 0x04)
        {
            ptsInterval = av_compare_ts(cur_pts_v, pOutFormatCtx->streams[outVideoIndex]->time_base,
                                cur_pts_a,pOutFormatCtx->streams[outAudioIndex]->time_base);
        }

        if(ptsInterval <= 0 || !(initFlag & 0x04)) //视频
        {
            if(!pVideoFifo || av_fifo_size(pVideoFifo) <= 0)
                continue;
            memset(frame_buf, 0, size);
            av_fifo_generic_read(pVideoFifo, frame_buf, size, NULL);
            av_image_fill_arrays(pOutFrame->data, pOutFrame->linesize, frame_buf, AV_PIX_FMT_YUV420P,
                   GlobalData::m_pConfigInfo->desktopWidth, GlobalData::m_pConfigInfo->desktopHeight, 1);

            if(!pH264CodecCtx)
                break;
            pOutFrame->format = pH264CodecCtx->pix_fmt;
            pOutFrame->width = GlobalData::m_pConfigInfo->desktopWidth;
            pOutFrame->height = GlobalData::m_pConfigInfo->desktopHeight;
            ret = avcodec_send_frame(pH264CodecCtx, pOutFrame);
            if (ret < 0)
            {
                qDebug() << "Failed to encode video frame.";
                //log->open(QIODevice::WriteOnly | QIODevice::Append);
                //log->write("Failed to encode video frame.\n");
                //log->close();
                break;
            }

            if(avcodec_receive_packet(pH264CodecCtx, pOutPkt) >= 0)
            {
                if(!pOutFormatCtx)
                    break;
                pOutPkt->stream_index = outVideoIndex;
                pOutPkt->pts = VideoFrameIndex * pOutFormatCtx->streams[0]->time_base.den / GlobalData::m_pConfigInfo->frameRate;
                pOutPkt->dts = pOutPkt->pts;
                cur_pts_v = pOutPkt->pts;

                pOutPkt->duration = ((pOutFormatCtx->streams[0]->time_base.den / pOutFormatCtx->streams[0]->time_base.num) / GlobalData::m_pConfigInfo->videoBitRate);

                ret = av_interleaved_write_frame(pOutFormatCtx, pOutPkt);
                if (ret < 0)
                {
                    qDebug() << "Failed to write video frame to url";
                    //log->open(QIODevice::WriteOnly | QIODevice::Append);
                    //log->write("Failed to write video frame to url.\n");
                    //log->close();
                    break;
                }
                //else qDebug() << "Send" << VideoFrameIndex << " video packet to url successfully. pts = " << cur_pts_v;
                VideoFrameIndex++;
                av_packet_unref(pOutPkt);
            }
        }
        else //音频
        {
            if(!pAudioFifo || av_audio_fifo_size(pAudioFifo) <= 0)
                continue;

            if(!pAudioCodecCtx)
                break;
            pOutFrame->nb_samples = pAudioCodecCtx->frame_size;
            pOutFrame->channel_layout = pAudioCodecCtx->channel_layout;
            pOutFrame->format = pAudioCodecCtx->sample_fmt;
            pOutFrame->sample_rate = pAudioCodecCtx->sample_rate;
            av_frame_get_buffer(pOutFrame, 0);

            av_audio_fifo_read(pAudioFifo, (void **)pOutFrame->data, pOutFrame->nb_samples);

            ret = avcodec_send_frame(pAudioCodecCtx, pOutFrame);
            if (ret < 0)
            {
                qDebug() << "Failed to encode audio frame.";
                //log->open(QIODevice::WriteOnly | QIODevice::Append);
                //log->write("Failed to encode audio frame.\n");
                //log->close();
                continue;
            }

            if (avcodec_receive_packet(pAudioCodecCtx, pOutPkt) >= 0)
            {
                if(!pOutFormatCtx)
                    break;
                pOutPkt->stream_index = outAudioIndex;

                uint64_t streamTimeBase = pOutFormatCtx->streams[outAudioIndex]->time_base.den;
                pOutPkt->pts = (uint64_t)(1024 * streamTimeBase * AudioFrameIndex) / pAudioCodecCtx->sample_rate;
                pOutPkt->dts = pOutPkt->pts;
                cur_pts_a = pOutPkt->pts;
                pOutPkt->duration = pAudioCodecCtx->frame_size;

                ret = av_interleaved_write_frame(pOutFormatCtx, pOutPkt);
                if (ret < 0)
                {
                    qDebug() << "Failed to write audio frame to url";
                    //log->open(QIODevice::WriteOnly | QIODevice::Append);
                    //log->write("Failed to write audio frame to url.\n");
                    //log->close();
                    break;
                }
                //else qDebug() << "Send" << AudioFrameIndex << " audio packet to url successfully. pts = " << cur_pts_a;
                AudioFrameIndex++;
                av_packet_unref(pOutPkt);
            }
        }
        av_frame_unref(pOutFrame);
    }
    av_write_trailer(pOutFormatCtx);

    av_frame_free(&pOutFrame);
    av_packet_free(&pOutPkt);
    delete frame_buf;
    return ret;
}

void Capture::run()
{
    QtConcurrent::run(this, &Capture::MergeCaptureStart);
    QtConcurrent::run(this, &Capture::MICCaptureStart);
    CaptureStart();
}

int Capture::interrupt_callback(void *p)
{
    Runner* r = (Runner*)p;

    if(r->lasttime > 0)
    {
        if(time(NULL) - r->lasttime > 3 && !input_runner.connected)
        {
            return 1;
        }
    }
    return 0;
}


