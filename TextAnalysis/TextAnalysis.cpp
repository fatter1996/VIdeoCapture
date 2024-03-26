#include "TextAnalysis.h"
#include "Universal.h"
#include <QDebug>
TextAnalysis::TextAnalysis()
{

}

TextAnalysis::~TextAnalysis()
{

}

void TextAnalysis::setStratTime()
{
    curTime = QDateTime::currentDateTime();
}

TextFrame* TextAnalysis::TextFrameUnpack(QByteArray dataArr)
{
    TextFrame* data = new TextFrame;
    int flag = 4;

    data->length = GlobalData::getBigEndian_2Bit(dataArr.mid(flag, 2));
    flag += 2;
    data->localCode = dataArr[flag++];
    data->desCode = dataArr[flag++];
    data->deviceType = dataArr[flag++];
    data->captureState = dataArr[flag++];
    data->textDisplayState = dataArr[flag++];
    data->time = GlobalData::getDateTimeByArray(dataArr.mid(flag, 7));
    flag += 7;
    data->point.setX(GlobalData::getBigEndian_2Bit(dataArr.mid(flag, 2)));
    flag += 2;
    data->point.setY(GlobalData::getBigEndian_2Bit(dataArr.mid(flag, 2)));
    flag += 2;
    data->duration = dataArr[flag++];
    data->textLenght = dataArr[flag++];
    data->textColor = dataArr[flag++];
    data->text = dataArr.mid(flag, data->textLenght);
    flag += data->textLenght;

    return data;
}

Dialogue* TextAnalysis::TextFrameToDialogue(TextFrame* frame)
{
    Dialogue* data = new Dialogue;
    data->start = frame->time.toTime_t() - curTime.toTime_t();
    data->end = data->start + frame->duration;

    QString color;
    switch (frame->textColor)
    {
    case 0:  color = "00ffffff"; break;
    case 1:  color = "00ff0000"; break;
    case 2:  color = "00ff6100"; break;
    case 3:  color = "00ffff00"; break;
    case 4:  color = "00228b22"; break;
    case 5:  color = "0087ceeb"; break;
    case 6:  color = "001e90ff"; break;
    case 7:  color = "00a020f0"; break;
    default: color = "00ffffff"; break;
    }

    data->text = QString("{\\c&%1&}{\\pos(%2,%3)}%4")
            .arg(color)
            .arg(frame->point.x())
            .arg(frame->point.y())
            .arg(QString::fromLocal8Bit(frame->text));

    return data;
}

