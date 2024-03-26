#include "CaptionsCompose.h"
#include <QDebug>

CaptionsCompose::CaptionsCompose()
{
    avdevice_register_all(); //注册设备
    avformat_network_init(); //加载socket库以及网络加密协议相关的库
}

CaptionsCompose::~CaptionsCompose()
{

}

int CaptionsCompose::VideoInit(QString inPutPath)
{
    int ret = 0;
    ret = avformat_open_input(&pInFormatCtx, inPutPath.toLatin1(), NULL, NULL);
    if(ret < 0)
    {
        qDebug() << "Couldn't open input stream.";
        goto error;
    }

    //获取流信息
    ret = avformat_find_stream_info(pInFormatCtx, NULL);
    if(ret < 0 )
    {
        qDebug() << "Couldn't find stream information.";
        goto error;
    }

    ret = VideoStreamInit();
    if(ret < 0 )
    {
        qDebug() << "Video stream init error.";
        goto error;
    }

    ret = AudioStreamInit();
    if(ret < 0 )
    {
        qDebug() << "Audio stream init error.";
        goto error;
    }

    return ret;

error:
    emit StopCompose();
    return ret;
}

int CaptionsCompose::VideoStreamInit()
{
    //获取流索引
    int ret = 0;
    for(int i = 0; i < pInFormatCtx->nb_streams; i++)
    {
        if(pInFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoIndex = i;
            break;
        }
    }
    if(videoIndex == -1)
    {
        qDebug() << "Didn't find a video stream.";
        return -1;
    }

    //为视频流创建对应的解码器
    pVideoCodec = avcodec_find_decoder(pInFormatCtx->streams[videoIndex]->codecpar->codec_id);
    if(pVideoCodec == NULL)
    {
        qDebug() << "Codec not found.";
        return -1;
    }

    pVideoCodecCtx = avcodec_alloc_context3(pVideoCodec);
    if (!pVideoCodecCtx)
    {
        qDebug() << "can't alloc codec context";
        return -1;
    }

    //复制解码器信息
    ret = avcodec_parameters_to_context(pVideoCodecCtx, pInFormatCtx->streams[videoIndex]->codecpar);
    if (ret < 0)
    {
        qDebug() << "Failed to copy context from input to output stream codec context.";
        return ret;
    }
    pVideoCodecCtx->codec_tag = 0;

    //打开解码器
    ret = avcodec_open2(pVideoCodecCtx, pVideoCodec, NULL);
    if(ret < 0)
    {
        qDebug() << "Could not open codec.";
        return ret;
    }

    pOutVideoCodec = avcodec_find_encoder(pVideoCodec->id);
    if (!pOutVideoCodec)
    {
        qDebug() << "Could not find h264 codec.";
        return -1;
    }

    pOutVideoCodecCtx= avcodec_alloc_context3(pOutVideoCodec);

    ret = avcodec_open2(pOutVideoCodecCtx, pOutVideoCodec, NULL);
    if ( ret< 0)
    {
        qDebug() << "Could not open video encoder.";
        return ret;
    }

    return ret;
}

int CaptionsCompose::AudioStreamInit()
{
    //获取流索引
    int ret = 0;
    for(int i = 0; i < pInFormatCtx->nb_streams; i++)
    {
        if(pInFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioIndex = i;
            break;
        }
    }
    if(audioIndex == -1)
    {
        qDebug() << "Didn't find a video stream.";
        return -1;
    }

    //为视频流创建对应的解码器
    pAudioCodec = avcodec_find_decoder(pInFormatCtx->streams[videoIndex]->codecpar->codec_id);
    if(pAudioCodec == NULL)
    {
        qDebug() << "Codec not found.";
        return -1;
    }

    pAudioCodecCtx = avcodec_alloc_context3(pAudioCodec);
    if (!pAudioCodecCtx)
    {
        qDebug() << "can't alloc codec context";
        return -1;
    }

    //复制解码器信息
    ret = avcodec_parameters_to_context(pAudioCodecCtx, pInFormatCtx->streams[videoIndex]->codecpar);
    if (ret < 0)
    {
        qDebug() << "Failed to copy context from input to output stream codec context.";
        return ret;
    }
    pAudioCodecCtx->codec_tag = 0;

    //打开解码器
    ret = avcodec_open2(pAudioCodecCtx, pAudioCodec, NULL);
    if(ret < 0)
    {
        qDebug() << "Could not open codec.";
        return ret;
    }

    return ret;
}


