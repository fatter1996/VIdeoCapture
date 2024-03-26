#ifndef ASSCAPTIONS_H
#define ASSCAPTIONS_H

#include <QThread>
#include <QFile>
#pragma execution_character_set("utf-8")

#define SCRIPT_VERSION "v4.00+"
#define SCRIPT_TIMER 100.0000


struct CaptionsStyle
{
    QString name = "Default"; //样式名称,区分大小写，不能包含逗号
    QString fontname= "宋体"; //字体名称，区分大小写
    int fontsize = 24; //字号
    QString primaryColour = "&H00000000"; //主体颜色（一般情况下文字的颜色）
    QString secondaryColour = "&H00ffffff"; //次要颜色
    QString outlineColor = "&H00ffffff"; //边框颜色
    QString backColour = "&H00c0c0c0"; //阴影颜色
    int bold = 0; //粗 体（ -1=开启，0=关闭）
    int italic = 0; //斜 体（ -1=开启，0=关闭）
    int underline = 0; //下划线 （ -1=开启，0=关闭）
    int strikeOut = 0; // 删除线（ -1=开启，0=关闭）
    int scaleX = 100; //横向缩放（单位 [%]，100即正常宽度）
    int scaleY = 100; //纵向缩放（单位 [%]，100即正常高度）
    int spacing = 4; //字间距（单位 [像素]，可用小数）
    double angle = 0.00; //旋转所基于的原点由 Alignment 定义（为角度）
    int borderStyle = 1; //边框样式（1=边框+阴影，3=不透明底框）
    int outline = 2; //边框宽度（单位 [像素]，可用小数）
    int shadow = 2; //阴影深度（单位 [像素]，可用小数，右下偏移）
    int alignment = 2; //对齐方式（同小键盘布局，决定了旋转/定位/缩放的参考点）
    int marginL = 20; //左边距（字幕距左边缘的距离，单位 [像素]，右对齐和中对齐时无效）
    int marginR = 20; //右边距（字幕距右边缘的距离，单位 [像素]，左对齐和中对齐时无效）
    int marginV = 20; //垂直边距（字幕距垂直边缘的距离，单位 [像素]，下对齐时表示到底部的距离、上对齐时表示到顶部的距离、中对齐时无效， 文本位于垂直中心）
    int encoding = 1; //编码（ 0=ANSI,1=默认,128=日文,134=简中,136=繁中）
};

struct Dialogue
{
    int layer = 0; //字幕图层（任意的整数，图层不同的两个字幕不被视为有冲突，图层号大的显示在上层）
    uint start = 0; //开始时间（格式0:00:00.00 [时:分:秒:百分数]，小时只有一位数）
    uint end = 0; //结束时间（格式0:00:00.00 [时:分:秒:百分数]，小时只有一位数）
    QString style = "Default"; //样式名称（引用[v4+ Styles]部分中的Name）
    QString name = ""; //角色名称（用于注释此句是谁讲的，字幕中不显示，可省略，缺省后保留逗号）
    int marginL = 20; //重新设定的左边距（为四位数的像素值，0000表示使用当前样式定义的值）
    int marginR = 20; //重新设定的右边距（为四位数的像素值，0000表示使用当前样式定义的值）
    int marginV = 20; //重新设定的垂直边距（为四位数的像素值，0000表示使用当前样式定义的值）
    QString effect = ""; //过渡效果（三选一，可省略，缺省后保留逗号；效果中各参数用分号分隔）
    QString text; //字幕文本
};

class ASSCaptions : public QThread
{
    Q_OBJECT
public:
    ASSCaptions(QObject* parent = nullptr);
    ~ASSCaptions();

    QString Create();
    void AddScriptInfo(int width, int height); //写入脚本的头部和总体信息
    void StylesInit(); //定义字幕样式
    void AddStyles(CaptionsStyle style); //添加字幕样式
    void EventsInit(); //事件部分初始化
    void AddEvents(Dialogue* event);
    void WriteEvents();

    QString TimeToString(uint time);

private:
    QFile* m_pFile = NULL;
    QVector<Dialogue*> eventList;
};

#endif // ASSCAPTIONS_H
