#include "AudioStreamCapture.h"
#include <QDebug>
#include <QSettings>
#include <QTextCodec>
#include <QAudioDeviceInfo>
#include <QMessageBox>

#pragma execution_character_set("utf-8")
AudioStreamCapture::AudioStreamCapture(QObject* parent)
{
    //读取配置文件
    QSettings *config = new QSettings("Config.ini", QSettings::IniFormat);
    config->setIniCodec(QTextCodec::codecForName("UTF-8"));
    pCaptureInfo = new AudioCaptureInfo;
    pCaptureInfo->rtmpAddr = config->value("/AudioCapture/OutUrl").toString();
    pCaptureInfo->SampleRate = config->value("/AudioCapture/SampleRate").toInt();
    pCaptureInfo->bitRate = config->value("/AudioCapture/BitRate").toInt();

    QVariant str = config->value("/MIC/MICName");
    pCaptureInfo->MICName = config->value("/MIC/MICName").toByteArray();
    pCaptureInfo->MICIndex = config->value("/MIC/MICIndex").toInt();

    avdevice_register_all(); //注册设备
    avformat_network_init(); //加载socket库以及网络加密协议相关的库
}

AudioStreamCapture::~AudioStreamCapture()
{

}

int AudioStreamCapture::MICCaptureInit()
{
    int ret = 0;
    //创建AVformatContext句柄
    pMICInFormatCtx = avformat_alloc_context();

    //获取dshow设备
    AVInputFormat* ifmt = av_find_input_format("dshow");
    QList<QAudioDeviceInfo> audioDevice = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    //打开输入设备
    AVDictionary* opts = nullptr;
    av_dict_set_int(&opts, "audio_buffer_size", 20, 0);

    QString urlString;
    if(pCaptureInfo->MICName == "")
    {
        if(pCaptureInfo->MICIndex >= 0)
            urlString = QString("audio=") + audioDevice.at(pCaptureInfo->MICIndex).deviceName();
        else urlString = QString("audio=") + audioDevice.at(0).deviceName();
    }
    else urlString = QString("audio=%1").arg(pCaptureInfo->MICName);

    ret = avformat_open_input(&pMICInFormatCtx, urlString.toStdString().c_str(), ifmt, &opts);
    if(ret < 0)
    {
        qDebug() << "Couldn't open MIC.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Couldn't open MIC.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    //获取流信息
    ret = avformat_find_stream_info(pMICInFormatCtx, NULL);
    if(ret < 0 )
    {
        qDebug() << "Couldn't find stream information.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Couldn't find stream information.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    //获取流索引
    for(int i = 0; i < pMICInFormatCtx->nb_streams; i++)
    {
        if(pMICInFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex = i;
            break;
        }
    }
    if(audioStreamIndex == -1)
    {
        qDebug() << "Didn't find a video stream.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Didn't find a video stream.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    //创建对应的解码器
    pMICCodec = avcodec_find_decoder(pMICInFormatCtx->streams[audioStreamIndex]->codecpar->codec_id);
    if(pMICCodec == NULL)
    {
        qDebug() << "Codec not found.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Codec not found.",QMessageBox::Yes);
        MyBox.exec();
        ret = -1;
        goto error;
    }

    pMICCodecCtx = avcodec_alloc_context3(pMICCodec);
    if (!pMICCodecCtx)
    {
        qDebug() << "can't alloc codec context";
        QMessageBox MyBox(QMessageBox::Question,"Title","can't alloc codec context",QMessageBox::Yes);
        MyBox.exec();
        ret = -1;
        goto error;
    }

    //复制解码器信息
    ret = avcodec_parameters_to_context(pMICCodecCtx, pMICInFormatCtx->streams[audioStreamIndex]->codecpar);
    if (ret < 0)
    {
        qDebug() << "Failed to copy context from input to output stream codec context.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Failed to copy context from input to output stream codec context.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    pMICCodecCtx->channel_layout = av_get_default_channel_layout(pMICCodecCtx->channels);
    pMICCodecCtx->codec_tag = 0;

    //打开解码器
    ret = avcodec_open2(pMICCodecCtx, pMICCodec, NULL);
    if(ret < 0)
    {
        qDebug() << "Could not open codec.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Could not open codec.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    return ret;

error:
    emit StopAudio();
    return ret;
}

int AudioStreamCapture::AudioFilterInit()
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

    AVRational time_base = pMICInFormatCtx->streams[audioStreamIndex]->time_base;
    sprintf_s(args, sizeof(args),
        "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%I64x",
        time_base.num, time_base.den, pMICCodecCtx->sample_rate,
        av_get_sample_fmt_name(pMICCodecCtx->sample_fmt), pMICCodecCtx->channel_layout);

    ret = avfilter_graph_create_filter(&pAudioBuffersrcCtx, abuffersrc, "in", args, NULL, filterGraph);
    if (ret < 0)
    {
        qDebug() << "Cannot create audio buffer source.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Cannot create audio buffer source.",QMessageBox::Yes);
        MyBox.exec();
        return ret;
    }

    /* buffer audio sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&pAudioBuffersinkCtx, abuffersink, "out", NULL, NULL, filterGraph);
    if (ret < 0)
    {
        qDebug() << "Cannot create audio buffer sink.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Cannot create audio buffer sink.",QMessageBox::Yes);
        MyBox.exec();
        return ret;
    }

    ret = av_opt_set_int_list(pAudioBuffersinkCtx, "sample_fmts", out_sample_fmts, -1, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        qDebug() << "Cannot set output sample format.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Cannot set output sample format.",QMessageBox::Yes);
        MyBox.exec();
        return ret;
    }

    ret = av_opt_set_int_list(pAudioBuffersinkCtx, "channel_layouts", out_channel_layouts, -1, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        qDebug() << "Cannot set output channel layout.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Cannot set output channel layout.",QMessageBox::Yes);
        MyBox.exec();
        return ret;
    }

    ret = av_opt_set_int_list(pAudioBuffersinkCtx, "sample_rates", out_sample_rates, -1, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0)
    {
        qDebug() << "Cannot set output sample rate.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Cannot set output sample rate.",QMessageBox::Yes);
        MyBox.exec();
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
        QMessageBox MyBox(QMessageBox::Question,"Title","Add audio graph described failed.",QMessageBox::Yes);
        MyBox.exec();
        return ret;
    }

    ret = avfilter_graph_config(filterGraph, NULL);
    if (ret < 0)
    {
        qDebug() << "Audio filter init failed.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Audio filter init failed.",QMessageBox::Yes);
        MyBox.exec();
        return ret;
    }

    av_buffersink_set_frame_size(pAudioBuffersinkCtx, 1024);
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    return 0;
}

int AudioStreamCapture::AudioRecodeInit()
{
    int ret = 0;
    //查找音频编码器
    pAudioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!pAudioCodec)
    {
        qDebug() << "Could not find h264 codec.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Could not find h264 codec.",QMessageBox::Yes);
        MyBox.exec();
        ret = -1;
        goto error;
    }

    //设置参数
    pAudioCodecCtx = avcodec_alloc_context3(pAudioCodec);
    pAudioCodecCtx->codec = pAudioCodec;
    pAudioCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    pAudioCodecCtx->sample_rate= 48000; //音频采样率
    pAudioCodecCtx->channels = 2;
    pAudioCodecCtx->channel_layout = av_get_default_channel_layout(pAudioCodecCtx->channels);
    pAudioCodecCtx->bit_rate = 1411200;
    pAudioCodecCtx->codec_tag = 0;
    pAudioCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    //打开音频编码器
    ret = avcodec_open2(pAudioCodecCtx, pAudioCodec, NULL);
    if ( ret< 0)
    {
        qDebug() << "Could not open audio encoder.";
        QMessageBox MyBox(QMessageBox::Question,"Title","Could not open audio encoder.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    return ret;

error:
    emit StopAudio();
    return ret;
}

int AudioStreamCapture::OutPutInit()
{
    int ret = 0;
    AVStream* outAudioStream = NULL;
    AVDictionary * options = NULL;
    av_dict_set(&options, "flvflags", "no_duration_filesize", 0);
    //分配输出ctx
    avformat_alloc_output_context2(&pOutFormatCtx, NULL, "flv", pCaptureInfo->rtmpAddr.toLatin1());
    if (!pOutFormatCtx)
    {
        qDebug() << "Could not create output context";
        QMessageBox MyBox(QMessageBox::Question,"Title","Could not create output context.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }

    //创建输出音频流
    outAudioStream = avformat_new_stream(pOutFormatCtx, pAudioCodec);
    if (!outAudioStream)
    {
        qDebug() << "Failed allocating output stream";
        QMessageBox MyBox(QMessageBox::Question,"Title","Failed allocating output stream.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }
    avcodec_copy_context(outAudioStream->codec, pAudioCodecCtx);
 // avcodec_parameters_from_context(outAudioStream->codecpar, pAudioCodecCtx);
    outAudioStream->codecpar->codec_tag = 0;
    outStreamIndex = outAudioStream->index;

    //打开输出url
    if (!(pOutFormatCtx->oformat->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&pOutFormatCtx->pb, pCaptureInfo->rtmpAddr.toLatin1(), AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            qDebug() << "Failed to open output url: " << pCaptureInfo->rtmpAddr << ". Error Code : " << ret;
            QMessageBox MyBox(QMessageBox::Question,"Title","Failed to open output url.",QMessageBox::Yes);
            MyBox.exec();
            goto error;
        }
    }

    //写文件头
    ret = avformat_write_header(pOutFormatCtx, options ? &options : NULL);
    if (ret < 0)
    {
        qDebug() << "Failed to write header. Error Code : " << ret;
        QMessageBox MyBox(QMessageBox::Question,"Title","Failed to write header.",QMessageBox::Yes);
        MyBox.exec();
        goto error;
    }
    return ret;

error:
    emit StopAudio();
    return ret;
}

int AudioStreamCapture::MICCaptureStart()
{
    int ret = 0;
    int frameIndex = 0;

    AVPacket* pMicPkt = av_packet_alloc();
    AVFrame* pMicFrame = av_frame_alloc();
    AVFrame* pOutFrame = av_frame_alloc();
    AVPacket* pOutPktA = av_packet_alloc();

    while (isRunning)
    {
        //从输入流读取一个packet
        ret = av_read_frame(pMICInFormatCtx, pMicPkt);
        if (ret < 0)
        {
            qDebug() << "Read frame failed.";
            QMessageBox MyBox(QMessageBox::Question,"Title","Read frame failed.",QMessageBox::Yes);
            MyBox.exec();
            return ret;
        }

        if (pMicPkt->stream_index == audioStreamIndex)
        {

            ret = avcodec_send_packet(pMICCodecCtx, pMicPkt);
            if (ret < 0)
            {
                qDebug() << "Decode audio frame failed.";
                QMessageBox MyBox(QMessageBox::Question,"Title","Decode audio frame failed.",QMessageBox::Yes);
                MyBox.exec();
                goto end;
            }
            ret = avcodec_receive_frame(pMICCodecCtx, pMicFrame);
            if(ret >= 0)
            {
                ret = av_buffersrc_add_frame_flags(pAudioBuffersrcCtx, pMicFrame, AV_BUFFERSRC_FLAG_PUSH);
                if (ret < 0)
                {
                    qDebug() << "Error while feeding the filtergraph.";
                    QMessageBox MyBox(QMessageBox::Question,"Title","Error while feeding the filtergraph.",QMessageBox::Yes);
                    MyBox.exec();
                    return ret;
                }

                while (1)
                //if(ret >= 0)
                {
                    ret = av_buffersink_get_frame_flags(pAudioBuffersinkCtx, pOutFrame, AV_BUFFERSINK_FLAG_NO_REQUEST );
                    if (ret < 0)
                    {
                        break;
                    }
                    ret = avcodec_send_frame(pAudioCodecCtx, pOutFrame);
                    if (ret < 0)
                    {
                        qDebug() << "Failed to encode frame.";
                        QMessageBox MyBox(QMessageBox::Question,"Title","Failed to encode frame.",QMessageBox::Yes);
                        MyBox.exec();
                        goto end;
                    }

                    ret = avcodec_receive_packet(pAudioCodecCtx, pOutPktA);
                    if(ret >= 0)
                    {
                        uint64_t streamTimeBase = pOutFormatCtx->streams[outStreamIndex]->time_base.den;
                        pOutPktA->pts = (uint64_t)(1024 * streamTimeBase * frameIndex) / pAudioCodecCtx->sample_rate;
                        pOutPktA->dts = pOutPktA->pts;
                        auto inputStream = pMICInFormatCtx->streams[audioStreamIndex];
                        auto outputStream = pOutFormatCtx->streams[outStreamIndex];
                        av_packet_rescale_ts(pOutPktA, inputStream->time_base, outputStream->time_base);
                        qDebug() << " pts = " << pOutPktA->pts ;
                        if(pOutPktA->pts < 0)
                            goto end;
                        frameIndex++;
                        pOutPktA->stream_index = outStreamIndex;

                        //frameList.append(*pOutPktA);
                       ret = av_interleaved_write_frame(pOutFormatCtx, pOutPktA);
                       if (ret < 0)
                       {
                           qDebug() << "Failed to write audio frame to url";
                           QMessageBox MyBox(QMessageBox::Question,"Title","Failed to write audio frame to url.",QMessageBox::Yes);
                           MyBox.exec();
                           break;
                       }
                        else qDebug() << "Send" << frameIndex << " audio packet to url successfully ";

                        av_packet_unref(pOutPktA);
                    }
                }
            }
        }
        av_frame_unref(pMicFrame);
        av_frame_unref(pOutFrame);
        av_packet_unref(pMicPkt);
    }
    av_packet_free(&pMicPkt);
    av_packet_free(&pOutPktA);
    //写文件尾
    av_write_trailer(pOutFormatCtx);

end:
    av_frame_free(&pMicFrame);
    av_frame_free(&pOutFrame);

    emit StopAudio();
    return ret;
}

int AudioStreamCapture::Init()
{
    int ret = 0;

    ret = MICCaptureInit();
    if(ret < 0)
    {
        return ret;
    }

    ret = AudioRecodeInit();
    if(ret < 0)
    {
        return ret;
    }

    ret = AudioFilterInit();
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

void AudioStreamCapture::CaptureStart()
{
    isRunning = true;
    this->start();
}

void AudioStreamCapture::CaptureStop()
{
    isRunning = false;
    emit StopAudio();
}

void AudioStreamCapture::run()
{
    MICCaptureStart();
}

void AudioStreamCapture::onCaptureStop()
{
    /* close input */
    if (pMICInFormatCtx)
        avformat_close_input(&pMICInFormatCtx);

    /* close codec context */
    if(pMICCodecCtx)
        avcodec_close(pMICCodecCtx);

    if(pAudioCodecCtx)
        avcodec_close(pAudioCodecCtx);

    /* close output */
    if (pOutFormatCtx)
    {
        if (!(pOutFormatCtx->oformat->flags & AVFMT_NOFILE))
            avio_close(pOutFormatCtx->pb);
        avformat_free_context(pOutFormatCtx);
    }
}

