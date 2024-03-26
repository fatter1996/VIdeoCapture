#include "VideoStreamCapture.h"
#include <QDebug>
#include <QSettings>
#include <QCameraInfo>
#include <QMessageBox>
#pragma execution_character_set("utf-8")

VideoStreamCapture::VideoStreamCapture(QObject* parent)
{
    //读取配置文件
    QSettings *config = new QSettings("Config.ini", QSettings::IniFormat);
    pCaptureInfo = new VideoCaptureInfo;
    pCaptureInfo->rtmpAddr = config->value("/VideoCapture/OutUrl").toString();
    pCaptureInfo->frameRate = config->value("/VideoCapture/FrameRate").toInt();
    pCaptureInfo->bitRate = config->value("/VideoCapture/BitRate").toInt();

    pCaptureInfo->desktopRange_x = config->value("/Desktop/Range_x").toInt();
    pCaptureInfo->desktopRange_y = config->value("/Desktop/Range_y").toInt();
    pCaptureInfo->desktopWidth = config->value("/Desktop/Width").toInt();
    pCaptureInfo->desktopHeight = config->value("/Desktop/Height").toInt();

    pCaptureInfo->cameraName = config->value("/Camera/CameraName").toString();
    pCaptureInfo->cameraIndex = config->value("/Camera/CameraIndex").toInt();
    pCaptureInfo->cameraWidth = config->value("/Camera/Width").toInt();
    pCaptureInfo->cameraHeight = config->value("/Camera/Height").toInt();
    pCaptureInfo->cameraPos_x = config->value("/Camera/Pos_x").toInt();
    pCaptureInfo->cameraPos_y = config->value("/Camera/Pos_y").toInt();

    avdevice_register_all(); //注册设备
    avformat_network_init(); //加载socket库以及网络加密协议相关的库
}

VideoStreamCapture::~VideoStreamCapture()
{

}

int VideoStreamCapture::DesktopCaptureInit()
{
    int ret = 0;
    //参数设置
    AVDictionary* options = NULL;
    av_dict_set_int(&options, "framerate", pCaptureInfo->frameRate, AV_DICT_MATCH_CASE); //帧率
    av_dict_set_int(&options, "offset_x", pCaptureInfo->desktopRange_x, AV_DICT_MATCH_CASE); //录制区域的起始坐标-x
    av_dict_set_int(&options, "offset_y", pCaptureInfo->desktopRange_y, AV_DICT_MATCH_CASE); //录制区域的起始坐标-y
    av_dict_set(&options, "video_size", QString("%1x%2").arg(pCaptureInfo->desktopWidth).arg(pCaptureInfo->desktopHeight).toLatin1(), AV_DICT_MATCH_CASE); //录制区域大小
    av_dict_set_int(&options, "draw_mouse", 1, AV_DICT_MATCH_CASE); //是否绘制鼠标

    //创建AVformatContext句柄
    pDesktopInFormatCtx = avformat_alloc_context();

    //获取gdigrab
    AVInputFormat* ifmt = av_find_input_format("gdigrab");

    //打开gidgrab,传入参数,获取封装格式相关信息
    ret = avformat_open_input(&pDesktopInFormatCtx, "desktop", ifmt, &options);
    if(ret < 0)
    {
        qDebug() << "Couldn't open input stream.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Couldn't open input stream.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    //获取流信息
    ret = avformat_find_stream_info(pDesktopInFormatCtx, NULL);
    if(ret < 0 )
    {
        qDebug() << "Couldn't find stream information.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Couldn't find stream information.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    //获取流索引
    for(int i = 0; i < pDesktopInFormatCtx->nb_streams; i++)
    {
        if(pDesktopInFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            desktopStreamIndex = i;
            break;
        }
    }
    if(desktopStreamIndex == -1)
    {
        qDebug() << "Didn't find a video stream.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Didn't find a video stream.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }
    //av_dump_format(pInFormatCtx, 0, "desktop", 0);

    //为gidgrab创建对应的解码器
    pDesktopCodec = avcodec_find_decoder(pDesktopInFormatCtx->streams[desktopStreamIndex]->codecpar->codec_id);
    if(pDesktopCodec == NULL)
    {
        qDebug() << "Codec not found.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Codec not found.",QMessageBox::Yes);
        MyBox.exec();
        ret = -1;
        goto error;
    }

    pDesktopCodecCtx = avcodec_alloc_context3(pDesktopCodec);
    if (!pDesktopCodecCtx)
    {
        qDebug() << "can't alloc codec context";
        QMessageBox MyBox(QMessageBox::Question,"Title","can't alloc codec context",QMessageBox::Yes);
        MyBox.exec();
        ret = -1;
        goto error;
    }

    //复制解码器信息
    ret = avcodec_parameters_to_context(pDesktopCodecCtx, pDesktopInFormatCtx->streams[desktopStreamIndex]->codecpar);
    if (ret < 0)
    {
        qDebug() << "Failed to copy context from input to output stream codec context.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Failed to copy context from input to output stream codec context.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }
    pDesktopCodecCtx->codec_tag = 0;

    //打开解码器
    ret = avcodec_open2(pDesktopCodecCtx, pDesktopCodec, NULL);
    if(ret < 0)
    {
        qDebug() << "Could not open codec.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Could not open codec.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    av_dict_free(&options);

    return ret;

error:
    emit Stop();
    return ret;
}

int VideoStreamCapture::CameraCaptureInit()
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
    if(pCaptureInfo->cameraName == "")
    {
        if(pCaptureInfo->cameraIndex >= 0)
            urlString = QString("video=") + cameras.at(pCaptureInfo->cameraIndex).description();
        else urlString = QString("video=") + cameras.at(0).description();
    }
    else urlString = QString("video=%1").arg(pCaptureInfo->cameraName);

    ret = avformat_open_input(&pCameraInFormatCtx, urlString.toStdString().c_str(), ifmt, NULL);
    //ret = avformat_open_input(&pCameraInFormatCtx, "video=720P USB Camera", ifmt, NULL);
    if(ret < 0)
    {
        qDebug() << "Couldn't open camera.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Couldn't open camera.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    //获取流信息
    ret = avformat_find_stream_info(pCameraInFormatCtx, NULL);
    if(ret < 0 )
    {
        qDebug() << "Couldn't find stream information.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Couldn't find stream information.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    //获取流索引
    for(int i = 0; i < pCameraInFormatCtx->nb_streams; i++)
    {
        if(pCameraInFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            cameraStreamIndex = i;
            break;
        }
    }
    if(cameraStreamIndex == -1)
    {
        qDebug() << "Didn't find a video stream.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Didn't find camera video stream.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    //创建对应的解码器
    pCameraCodec = avcodec_find_decoder(pCameraInFormatCtx->streams[cameraStreamIndex]->codecpar->codec_id);
    if(pCameraCodec == NULL)
    {
        qDebug() << "Codec not found.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Codec not found.",QMessageBox::Yes);
        MyBox.exec();
        ret = -1;
        goto error;
    }

    pCameraCodecCtx = avcodec_alloc_context3(pCameraCodec);
    if (!pCameraCodecCtx)
    {
        qDebug() << "can't alloc codec context";
        QMessageBox MyBox(QMessageBox::Question,"Title","can't alloc codec context",QMessageBox::Yes);
        MyBox.exec();
        ret = -1;
        goto error;
    }

    //复制解码器信息
    ret = avcodec_parameters_to_context(pCameraCodecCtx, pCameraInFormatCtx->streams[cameraStreamIndex]->codecpar);
    if (ret < 0)
    {
        qDebug() << "Failed to copy context from input to output stream codec context.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Failed to copy context from input to output stream codec context.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }
    pCameraCodecCtx->codec_tag = 0;

    //打开解码器
    ret = avcodec_open2(pCameraCodecCtx, pCameraCodec, NULL);
    if(ret < 0)
    {
        qDebug() << "Could not open codec.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Could not open codec.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    return ret;

error:
    emit Stop();
    return ret;
}

int VideoStreamCapture::VideoFilterInit()
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
            .arg(pCaptureInfo->desktopWidth).arg(pCaptureInfo->desktopHeight)
            .arg(pCaptureInfo->cameraWidth).arg(pCaptureInfo->cameraHeight)
            .arg(pCaptureInfo->cameraPos_x).arg(pCaptureInfo->cameraPos_y);

    //创建in - 桌面视频流
    snprintf(args[0], sizeof(args[0]), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                        pCaptureInfo->desktopWidth, pCaptureInfo->desktopHeight, AV_PIX_FMT_YUV420P, 1, pCaptureInfo->frameRate, 0, 1);

    ret = avfilter_graph_create_filter(&pVideoBuffersrcCtx[0], buffersrc[0], "in0", args[0], NULL, filter_graph);
    if (ret < 0)
    {
        qDebug() << "Cannot create buffer source";
        QMessageBox MyBox(QMessageBox::Question,"Title","Cannot create buffer source",QMessageBox::Yes);
        MyBox.exec();
        return ret;
    }

    //创建in - 摄像头视频流
    snprintf(args[1], sizeof(args[1]), "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
            pCaptureInfo->cameraWidth, pCaptureInfo->cameraHeight, AV_PIX_FMT_YUV420P, 1, pCaptureInfo->frameRate, 0, 1);

    ret = avfilter_graph_create_filter(&pVideoBuffersrcCtx[1], buffersrc[1], "in1", args[1], NULL, filter_graph);
    if (ret < 0)
    {
        qDebug() << "Cannot create buffer source.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Cannot create buffer source",QMessageBox::Yes);
        MyBox.exec();
        return ret;
    }

    //创建 out
    buffersink_params->pixel_fmts = pix_fmts;
    ret = avfilter_graph_create_filter(&pVideoBuffersinkCtx, buffersink, "out", NULL, buffersink_params, filter_graph);
    av_free(buffersink_params);
    if (ret < 0)
    {
        qDebug() << "Cannot create buffer sink.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Cannot create buffer sink.",QMessageBox::Yes);
        MyBox.exec();
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
        QMessageBox MyBox(QMessageBox::Question,"Title","Add video graph described failed.",QMessageBox::Yes);
        MyBox.exec();
        return ret;
    }

    ret = avfilter_graph_config(filter_graph, NULL);
    if (ret < 0)
    {
        qDebug() << "Video filter init failed.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Video filter init failed.",QMessageBox::Yes);
        MyBox.exec();
        return ret;
    }

    avfilter_inout_free(&input);
    avfilter_inout_free(outputs);

    return 0;
}

int VideoStreamCapture::H264RecodeInit()
{
    int ret = 0;
    AVDictionary* options = NULL;
    //查找H264编码器
    pH264Codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!pH264Codec)
    {
        qDebug() << "Could not find h264 codec.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Could not find h264 codec.",QMessageBox::Yes);
        MyBox.exec();
        ret = -1;
        goto error;
    }

    // 设置参数
    pH264CodecCtx = avcodec_alloc_context3(pH264Codec);
    pH264CodecCtx->codec_id = AV_CODEC_ID_H264;
    pH264CodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pH264CodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    pH264CodecCtx->width = pCaptureInfo->desktopWidth;
    pH264CodecCtx->height = pCaptureInfo->desktopHeight;
    pH264CodecCtx->time_base.num = 1;
    pH264CodecCtx->time_base.den = pCaptureInfo->frameRate;	//帧率（即一秒钟多少张图片）
    pH264CodecCtx->bit_rate = pCaptureInfo->bitRate;	//比特率（调节这个大小可以改变编码后视频的质量）
    pH264CodecCtx->rc_max_rate = pCaptureInfo->bitRate;
    pH264CodecCtx->rc_min_rate = pCaptureInfo->bitRate;
    pH264CodecCtx->gop_size = 2;
    pH264CodecCtx->qmin = 10;
    pH264CodecCtx->qmax = 51;
    pH264CodecCtx->max_b_frames = 0;
    pH264CodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER | AV_CODEC_FLAG_LOW_DELAY;

    //打开H.264编码器
    av_dict_set(&options, "preset", "superfast", 0);
    av_dict_set(&options, "tune", "zerolatency", 0);//实现实时编码
    av_dict_set(&options, "rtbufsize", "2000k", 0);
    av_dict_set(&options, "start_time_realtime", 0, 0);
    av_dict_set(&options, "crf", "18", 0);
    av_dict_set(&options, "level", "4",0);

    ret = avcodec_open2(pH264CodecCtx, pH264Codec, &options);
    if ( ret< 0)
    {
        qDebug() << "Could not open video encoder.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Could not open video encoder.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    return ret;

error:
    emit Stop();
    return ret;
}

int VideoStreamCapture::OutPutInit()
{
    int ret = 0;
    AVStream* outVideoStream = NULL;
    AVDictionary * options = NULL;
    av_dict_set(&options, "flvflags", "no_duration_filesize", 0);
    //分配输出ctx
    avformat_alloc_output_context2(&pOutFormatCtx, NULL, "flv", pCaptureInfo->rtmpAddr.toLatin1());
    if (!pOutFormatCtx)
    {
        qDebug() << "Could not create output context";
        QMessageBox MyBox(QMessageBox::Question,"Title","Could not create output context",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    //创建输出视频流
    outVideoStream = avformat_new_stream(pOutFormatCtx, pH264Codec);
    if (!outVideoStream)
    {
        qDebug() << "Failed allocating output stream";
        QMessageBox MyBox(QMessageBox::Question,"Title","Failed allocating output stream",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }
    avcodec_parameters_from_context(outVideoStream->codecpar, pH264CodecCtx);
    outVideoStream->codecpar->codec_tag = 0;
    outStreamIndex = outVideoStream->index;

    //打开输出url
    if (!(pOutFormatCtx->oformat->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&pOutFormatCtx->pb, pCaptureInfo->rtmpAddr.toLatin1(), AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            qDebug() << "Failed to open output url: " << pCaptureInfo->rtmpAddr << ". Error Code : " << ret;
            QMessageBox MyBox(QMessageBox::Question,"Title","Failed to open output url",QMessageBox::Yes);
            MyBox.exec();
            goto error;
        }
    }

    //写文件头
    ret = avformat_write_header(pOutFormatCtx, options ? &options : NULL);
    if (ret < 0)
    {
        qDebug() << "Failed to write header. Error Code : " << ret;
        QMessageBox MyBox(QMessageBox::Question,"Title","Failed to write header. Error Code",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }
    return ret;

error:
    emit Stop();
    return ret;
}

int VideoStreamCapture::MergeCaptureStart()
{
    int ret = 0;
    int frameIndex = 0;

    SwsContext* pDesktopImgConvertCtx = NULL;
    AVPacket* pDesktopPkt = av_packet_alloc();
    AVFrame* pDesktopFrame = av_frame_alloc();
    AVFrame* pDesktopFrameYUV = av_frame_alloc();

    SwsContext* pCameraImgConvertCtx = NULL;
    AVPacket* pCameraPkt = av_packet_alloc();
    AVFrame* pCameraFrame = av_frame_alloc();
    AVFrame* pCameraFrameYUV = av_frame_alloc();

    AVPacket* pOutPkt = av_packet_alloc();
    AVFrame* pFrameFilt = av_frame_alloc();

    //桌面视频流Frame初始化
    unsigned char* DesktopBuffer = (unsigned char*)av_malloc(av_image_get_buffer_size(
                              AV_PIX_FMT_YUV420P, pDesktopCodecCtx->width, pDesktopCodecCtx->height, 1));

    av_image_fill_arrays(pDesktopFrameYUV->data, pDesktopFrameYUV->linesize, DesktopBuffer,
            AV_PIX_FMT_YUV420P, pDesktopCodecCtx->width, pDesktopCodecCtx->height, 1);

    pDesktopImgConvertCtx = sws_getContext(pDesktopCodecCtx->width, pDesktopCodecCtx->height,
            pDesktopCodecCtx->pix_fmt, pDesktopCodecCtx->width, pDesktopCodecCtx->height,
            AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    //摄像头视频流Frame初始化
    unsigned char* CameraBuffer = (unsigned char*)av_malloc(av_image_get_buffer_size(
                              AV_PIX_FMT_YUV420P, pCameraCodecCtx->width, pCameraCodecCtx->height, 1));

    av_image_fill_arrays(pCameraFrameYUV->data, pCameraFrameYUV->linesize, CameraBuffer,
            AV_PIX_FMT_YUV420P, pCameraCodecCtx->width, pCameraCodecCtx->height, 1);

    pCameraImgConvertCtx = sws_getContext(pCameraCodecCtx->width, pCameraCodecCtx->height,
            pCameraCodecCtx->pix_fmt, pCameraCodecCtx->width, pCameraCodecCtx->height,
            AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

    while (isRunning)
    {
        //写入视频frame
        ret = GetDesktopFrame(pDesktopPkt, pDesktopFrame, pDesktopFrameYUV, pDesktopImgConvertCtx);
        if (ret < 0)
        {
            qDebug() << "Get desktop frame failed.";
            QMessageBox MyBox(QMessageBox::Question,"Title","Get desktop frame failed.",QMessageBox::Yes);
            MyBox.exec();
            goto end;
        }

        ret = GetCameraFrame(pCameraPkt, pCameraFrame, pCameraFrameYUV, pCameraImgConvertCtx);
        if (ret < 0)
        {
            qDebug() << "Get camera frame failed.";
            QMessageBox MyBox(QMessageBox::Question,"Title","Get camera frame failed.",QMessageBox::Yes);
            MyBox.exec();
            goto end;
        }

        while (1)
        {
            ret = av_buffersink_get_frame(pVideoBuffersinkCtx, pFrameFilt);
            if (ret < 0)
            {
                break;
            }

            ret = avcodec_send_frame(pH264CodecCtx, pFrameFilt);
            if (ret < 0)
            {
                qDebug() << "Failed to encode frame.";
                QMessageBox MyBox(QMessageBox::Question,"Title","Failed to encode frame.",QMessageBox::Yes);
                MyBox.exec();
                goto end;
            }

            ret = avcodec_receive_packet(pH264CodecCtx, pOutPkt);
            if(ret >= 0)
            {
                // 设置输出DTS,PTS
                pOutPkt->pts = frameIndex * (pOutFormatCtx->streams[0]->time_base.den) / pOutFormatCtx->streams[0]->time_base.num / pCaptureInfo->frameRate;
                pOutPkt->dts = pOutPkt->pts;
                qDebug() << " pts = " << pOutPkt->pts << " duration = " << pOutPkt->duration;
                frameIndex++;

                ret = av_interleaved_write_frame(pOutFormatCtx, pOutPkt);
                if (ret < 0)
                {
                    qDebug() << "Failed to write video frame to url";
                    QMessageBox MyBox(QMessageBox::Question,"Title","Failed to write video frame to url",QMessageBox::Yes);
                    MyBox.exec();
                    break;
                }
                else qDebug() << "Send" << frameIndex << " video packet to url successfully ";

                av_packet_unref(pOutPkt);
            }
        }
        av_frame_unref(pFrameFilt);
        av_frame_unref(pDesktopFrame);
        av_frame_unref(pCameraFrame);

        av_packet_unref(pDesktopPkt);
        av_packet_unref(pCameraPkt);
    }
    av_packet_free(&pDesktopPkt);
    av_packet_free(&pCameraPkt);
    av_packet_free(&pOutPkt);

    av_write_trailer(pOutFormatCtx);

end:

    av_frame_free(&pDesktopFrame);
    av_frame_free(&pDesktopFrameYUV);
    av_frame_free(&pCameraFrame);
    av_frame_free(&pCameraFrameYUV);
    av_frame_free(&pFrameFilt);

    emit Stop();
    return ret;
}

int VideoStreamCapture::GetDesktopFrame(AVPacket* packet, AVFrame* pFrame, AVFrame* pFrameYUV, SwsContext* pImgConvertCtx)
{
    int ret = 0;
    //从输入流读取一个packet
    ret = av_read_frame(pDesktopInFormatCtx, packet);
    if (ret < 0)
    {
        qDebug() << "Read desktop frame failed.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Read desktop frame failed",QMessageBox::Yes);
        MyBox.exec();
        return ret;
    }

    if (packet->stream_index == desktopStreamIndex)
    {
        ret = avcodec_send_packet(pDesktopCodecCtx, packet);
        if (ret < 0)
        {
            qDebug() << "Decode packet failed.";
            QMessageBox MyBox(QMessageBox::Question,"Title","Decode packet failed.",QMessageBox::Yes);
            MyBox.exec();
            return ret;
        }

        ret = avcodec_receive_frame(pDesktopCodecCtx, pFrame);
        if (ret >= 0)
        {

            sws_scale(pImgConvertCtx,(const unsigned char* const*)pFrame->data, pFrame->linesize,
                      0, pDesktopCodecCtx->height, pFrameYUV->data,pFrameYUV->linesize);

            pFrameYUV->format = AV_PIX_FMT_YUV420P;
            pFrameYUV->width = pDesktopCodecCtx->width;
            pFrameYUV->height = pDesktopCodecCtx->height;

            ret = av_buffersrc_add_frame_flags(pVideoBuffersrcCtx[0], pFrameYUV, AV_BUFFERSRC_FLAG_KEEP_REF );
            if (ret < 0)
            {
                qDebug() << "Error while feeding the filtergraph.";
                QMessageBox MyBox(QMessageBox::Question,"Title","Error while feeding the filtergraph.",QMessageBox::Yes);
                MyBox.exec();
                return ret;
            }
        }
    }
    return ret;
}

int VideoStreamCapture::GetCameraFrame(AVPacket* packet, AVFrame* pFrame, AVFrame* pFrameYUV, SwsContext* pImgConvertCtx)
{
    int ret = 0;
    //从输入流读取一个packet
    ret = av_read_frame(pCameraInFormatCtx, packet);
    if (ret < 0)
    {
        qDebug() << "Read camera frame failed.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Read camera frame failed.",QMessageBox::Yes);
        MyBox.exec();
        return ret;
    }

    if (packet->stream_index == cameraStreamIndex)
    {
        ret = avcodec_send_packet(pCameraCodecCtx, packet);
        if (ret < 0)
        {
            qDebug() << "Decode packet failed.";
            QMessageBox MyBox(QMessageBox::Question,"Title","Decode packet failed.",QMessageBox::Yes);
            MyBox.exec();
            return ret;
        }

        ret = avcodec_receive_frame(pCameraCodecCtx, pFrame);
        if (ret >= 0)
        {

            sws_scale(pImgConvertCtx,(const unsigned char* const*)pFrame->data, pFrame->linesize,
                      0, pCameraCodecCtx->height, pFrameYUV->data,pFrameYUV->linesize);

            pFrameYUV->format = AV_PIX_FMT_YUV420P;
            pFrameYUV->width = pCameraCodecCtx->width;
            pFrameYUV->height = pCameraCodecCtx->height;

            ret = av_buffersrc_add_frame_flags(pVideoBuffersrcCtx[1], pFrameYUV, AV_BUFFERSRC_FLAG_KEEP_REF);
            if (ret < 0)
            {
                qDebug() << "Error while feeding the filtergraph.";
                QMessageBox MyBox(QMessageBox::Question,"Title","Error while feeding the filtergraph.",QMessageBox::Yes);
                MyBox.exec();
                return ret;
            }
        }
    }
    return ret;
}


int VideoStreamCapture::Init()
{
    int ret = 0;

    ret = DesktopCaptureInit();
    if(ret < 0)
    {
        return ret;
    }

    ret = CameraCaptureInit();
    if(ret < 0)
    {
        return ret;
    }


    ret = H264RecodeInit();
    if(ret < 0)
    {
        return ret;
    }

    ret = VideoFilterInit();
    if(ret < 0)
    {
        return ret;
    }
    ret = OutPutInit();
    if(ret < 0)
    {
        return ret;
    }

    return ret;
}

void VideoStreamCapture::CaptureStart()
{
    isRunning = true;
    this->start();
}

void VideoStreamCapture::CaptureStop()
{
    isRunning = false;
    emit Stop();
}

void VideoStreamCapture::run()
{
    MergeCaptureStart();
}


void VideoStreamCapture::onCaptureStop()
{
    /* close input */
    if (pDesktopInFormatCtx)
        avformat_close_input(&pDesktopInFormatCtx);
    if (pCameraInFormatCtx)
        avformat_close_input(&pCameraInFormatCtx);

    /* close codec context */
    if(pDesktopCodecCtx)
        avcodec_close(pDesktopCodecCtx);
    if(pCameraCodecCtx)
        avcodec_close(pCameraCodecCtx);
    if(pH264CodecCtx)
        avcodec_close(pH264CodecCtx);

    /* close output */
    if (pOutFormatCtx)
    {
        if (!(pOutFormatCtx->oformat->flags & AVFMT_NOFILE))
            avio_close(pOutFormatCtx->pb);
        avformat_free_context(pOutFormatCtx);
    }
}
