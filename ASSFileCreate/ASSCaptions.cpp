#include "ASSCaptions.h"
#include <QDateTime>
#include <QtConcurrent>

ASSCaptions::ASSCaptions(QObject* parent)
{


}

ASSCaptions::~ASSCaptions()
{
    m_pFile->close();
    if(m_pFile)
        delete m_pFile;
}

QString ASSCaptions::Create()
{
    QString fileName = QString("Captions_%1.ass").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));

    m_pFile = new QFile(fileName);

    m_pFile->open(QIODevice::WriteOnly | QIODevice::Append);

    return fileName;
}

//写入脚本的头部和总体信息
void ASSCaptions::AddScriptInfo(int width, int height)
{
    m_pFile->write("[Script Info]\n");
    m_pFile->write(QString("ScriptType: %1\n").arg(SCRIPT_VERSION).toStdString().c_str()); //脚本版本,ass为v4.00+
    m_pFile->write("Collisions: Normal\n"); //字幕防重叠模式
    m_pFile->write(QString("PlayResX: %1\n").arg(width).toStdString().c_str()); //渲染范围宽度,一般与视频范围相同
    m_pFile->write(QString("PlayResY: %1\n").arg(height).toStdString().c_str()); //渲染范围高度,一般与视频范围相同
    m_pFile->write("Timer: 100.0000\n"); //脚本的计时器速度(百分数)
    m_pFile->write("WrapStyle: 0\n"); //默认换行方式
    m_pFile->write("\n");
}

//定义字幕样式
void ASSCaptions::StylesInit()
{
    m_pFile->write("[v4+ Styles]\n");
    //写入格式行
    m_pFile->write("Format: "
                  "Name, " //样式名称,区分大小写，不能包含逗号
                  "Fontname, " //字体名称，区分大小写
                  "Fontsize, " //字号
                  "PrimaryColour, " //主体颜色（一般情况下文字的颜色）
                  "SecondaryColour, " //次要颜色
                  "OutlineColor , " //边框颜色
                  "BackColour, " //阴影颜色
                  "Bold, " //粗 体（ -1=开启，0=关闭）
                  "Italic, " //斜 体（ -1=开启，0=关闭）
                  "Underline, " //下划线 （ -1=开启，0=关闭）
                  "StrikeOut, " // 删除线（ -1=开启，0=关闭）
                  "ScaleX, " //横向缩放（单位 [%]，100即正常宽度）
                  "ScaleY, " //纵向缩放（单位 [%]，100即正常高度）
                  "Spacing, " //字间距（单位 [像素]，可用小数）
                  "Angle, " //旋转所基于的原点由 Alignment 定义（为角度）
                  "BorderStyle, " //边框样式（1=边框+阴影，3=不透明底框）
                  "Outline, " //边框宽度（单位 [像素]，可用小数）
                  "Shadow, " //阴影深度（单位 [像素]，可用小数，右下偏移）
                  "Alignment, " //对齐方式（同小键盘布局，决定了旋转/定位/缩放的参考点）
                  "MarginL, " //左边距（字幕距左边缘的距离，单位 [像素]，右对齐和中对齐时无效）
                  "MarginR, " //右边距（字幕距右边缘的距离，单位 [像素]，左对齐和中对齐时无效）
                  "MarginV, " //垂直边距（字幕距垂直边缘的距离，单位 [像素]，下对齐时表示到底部的距离、上对齐时表示到顶部的距离、中对齐时无效， 文本位于垂直中心）
                  "Encoding\n"); //编码（ 0=ANSI,1=默认,128=日文,134=简中,136=繁中）
}

void ASSCaptions::AddStyles(CaptionsStyle style)
{
    //写入样式行
    m_pFile->write(QString("Style: %1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,%16,%17,%18,%19,%20,%21,%22,%23\n")
            .arg(style.name).arg(style.fontname).arg(style.fontsize)
            .arg(style.primaryColour).arg(style.secondaryColour)
            .arg(style.outlineColor).arg(style.backColour)
            .arg(style.bold).arg(style.italic)
            .arg(style.underline).arg(style.strikeOut)
            .arg(style.scaleX).arg(style.scaleY)
            .arg(style.spacing).arg(QString::number(style.angle, 'f', 2))
            .arg(style.borderStyle).arg(style.outline)
            .arg(style.shadow).arg(style.alignment)
            .arg(style.marginL).arg(style.marginR).arg(style.marginV)
            .arg(style.encoding).toStdString().c_str());
}

void ASSCaptions::EventsInit()
{
     m_pFile->write("[Events]\n");
     //写入格式行
     m_pFile->write("Format: "
                   "Layer, " //字幕图层（任意的整数，图层不同的两个字幕不被视为有冲突，图层号大的显示在上层）
                   "Start , " //开始时间（格式0:00:00.00 [时:分:秒:百分数]，小时只有一位数）
                   "End , " //结束时间（格式0:00:00.00 [时:分:秒:百分数]，小时只有一位数）
                   "Style , " //样式名称（引用[v4+ Styles]部分中的Name）
                   "Name , " //角色名称（用于注释此句是谁讲的，字幕中不显示，可省略，缺省后保留逗号）
                   "MarginL  , " //重新设定的左边距（为四位数的像素值，0000表示使用当前样式定义的值）
                   "MarginR , " //重新设定的右边距（为四位数的像素值，0000表示使用当前样式定义的值）
                   "MarginV , " //重新设定的垂直边距（为四位数的像素值，0000表示使用当前样式定义的值）
                   "Effect, " //过渡效果（三选一，可省略，缺省后保留逗号；效果中各参数用分号分隔）
                   "Text\n"); //字幕文本
}

void ASSCaptions::AddEvents(Dialogue* event)
{
    eventList.append(event);
}

void ASSCaptions::WriteEvents()
{
    Dialogue* event = NULL;
    Dialogue* event2 = NULL;
    for(QVector<Dialogue*>::iterator it = eventList.begin(); it != eventList.end(); ++it)
    {
        event = *it;
        if(it + 1 != eventList.end())
        {
            event2 = *(it +1);
            if(event->end > event2->start)
            {
                event->end = event2->start;
            }
            if(event->start == event->end)
            {
                event->end += 1;
            }
        }
        m_pFile->write(QString("Dialogue: %1,%2,%3,%4,%5,%6,%7,%8,%9,%10\n")
                      .arg(event->layer)
                      .arg(TimeToString(event->start))
                      .arg(TimeToString(event->end))
                      .arg(event->style).arg(event->name)
                      .arg(event->marginL).arg(event->marginR).arg(event->marginV)
                      .arg(event->effect).arg(event->text).toStdString().c_str());
    }
}

QString ASSCaptions::TimeToString(uint time)
{
    return QString("%1:%2:%3.00")
           .arg(time / 3600)
           .arg(time / 60 % 60, 2, 10, QLatin1Char('0'))
           .arg(time % 60, 2, 10, QLatin1Char('0'));
}
