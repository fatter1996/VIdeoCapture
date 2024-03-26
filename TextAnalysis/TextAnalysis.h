#ifndef TEXTANALYSIS_H
#define TEXTANALYSIS_H
#pragma execution_character_set("utf-8")
#include <QByteArray>
#include <QObject>
#include <QDateTime>
#include "ASSFileCreate/ASSCaptions.h"

struct TextFrame
{
    const QByteArray frameHead = QByteArray::fromHex("EFEFEFEF"); //头标记
    int length = 0; //帧长度
    int localCode = 0; //本机地址码
    int desCode = 0; //目标地址码
    unsigned char deviceType = 0; //车站标志/软件类型
    unsigned char captureState = 0; //录制状态,0xA5:开始录制，0X5A：结束录制，0xAA录制中状态，0x00其他状态
    unsigned char textDisplayState= 0; //文字显示状态,0xB5:显示文字，0X5B：不显示文字，0x00其他
    QDateTime time; //时间;
    QPoint point = { 0,0 }; //坐标
    int duration = 0; //时长
    int textLenght = 0; //文字长度
    unsigned int textColor = 0; //文字颜色
    QByteArray text; //显示内容
    const QByteArray frameEnd = QByteArray::fromHex("FEFEFEFE"); //尾标记
};

class TextAnalysis
{
public:
    TextAnalysis();
    ~TextAnalysis();

    void setStratTime();
    TextFrame* TextFrameUnpack(QByteArray dataArr);
    Dialogue* TextFrameToDialogue(TextFrame* frame);

private:
    QDateTime curTime;
};

#endif // TEXTANALYSIS_H
