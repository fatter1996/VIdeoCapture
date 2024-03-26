#include "VoiceDialogWnd.h"
#include "ui_VoiceDialogWnd.h"
#include "Universal.h"
#include <QEvent>
#include <QPainter>

#include <QtMath>
#include <QGraphicsDropShadowEffect>
#pragma execution_character_set("utf-8")

VoiceDialogWnd::VoiceDialogWnd(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VoiceDialogWnd)
{
    this->setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    ui->setupUi(this);
    ui->agreeBtn->setCursor(QCursor(Qt::PointingHandCursor));
    ui->disAgreeBtn->setCursor(QCursor(Qt::PointingHandCursor));
    ui->closeBtn->setCursor(QCursor(Qt::PointingHandCursor));
    ui->closeBtn_2->setCursor(QCursor(Qt::PointingHandCursor));

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    //设置阴影距离
    shadow->setOffset(0, 0);
    //设置阴影颜色
    shadow->setColor(Qt::black);
    //设置阴影圆角
    shadow->setBlurRadius(10);
    //给嵌套QWidget设置阴影
    ui->widget->setGraphicsEffect(shadow);

}

VoiceDialogWnd::~VoiceDialogWnd()
{
    delete ui;
}

void VoiceDialogWnd::paintEvent(QPaintEvent *event)
{
    //绘制阴影
     QPainterPath path;
     path.setFillRule(Qt::WindingFill);
     path.addRoundedRect(5, 5, this->width() - 5 * 2, this->height() - 5 * 2, 3, 3);
     QPainter painter(this);
     painter.setRenderHint(QPainter::Antialiasing, true);
     painter.fillPath(path, QBrush(Qt::white));

     QColor color(Qt::gray);
     for (int i = 0; i < 5; i++)
     {
         QPainterPath path;
         path.setFillRule(Qt::WindingFill);
         path.addRoundedRect(5 - i, 5 - i, this->width() - (5 - i) * 2, this->height() - (5 - i) * 2, 3 + i, 3 + i);
         color.setAlpha(80 - qSqrt(i) * 40);
         painter.setPen(color);
         painter.drawPath(path);
     }
}


void VoiceDialogWnd::WaitReply()
{
    ui->agreeBtn->hide();
    ui->disAgreeBtn->hide();
    ui->closeBtn->show();
    ui->label->setText(QString("正在呼叫"));
    state = 1;
}

void VoiceDialogWnd::NewCall(QString name)
{
    ui->agreeBtn->show();
    ui->disAgreeBtn->show();
    ui->closeBtn->hide();
    ui->label->setText(QString("%1正在请求通话").arg(name));
    timeLenght = 0;
    timerId2 = startTimer(30000);
    state = 2;
}

void VoiceDialogWnd::StartChat()
{
    ui->agreeBtn->hide();
    ui->disAgreeBtn->hide();
    ui->closeBtn->show();
    timeLenght = 0;
    ui->label->setText(QString("正在通话 %1:%2:%3").arg(timeLenght / 3600).arg(timeLenght % 3600 / 60).arg(timeLenght % 60));
    timerId = startTimer(1000);
    killTimer(timerId2);
    state = 3;
}

void VoiceDialogWnd::ChatEnd()
{
    killTimer(timerId2);
    killTimer(timerId);
    ui->agreeBtn->hide();
    ui->disAgreeBtn->hide();
    ui->closeBtn->hide();
    ui->label->setText(QString("通话结束 %1:%2:%3").arg(timeLenght / 3600).arg(timeLenght % 3600 / 60).arg(timeLenght % 60));
    state = 4;
}

void VoiceDialogWnd::TimeOut()
{
    ui->agreeBtn->hide();
    ui->disAgreeBtn->hide();
    ui->closeBtn->hide();
    ui->label->setText(QString("呼叫超时"));
    state = 5;
}

void VoiceDialogWnd::DisAgree()
{
    ui->agreeBtn->hide();
    ui->disAgreeBtn->hide();
    ui->closeBtn->hide();
    ui->label->setText(QString("对方已拒绝通话"));
    state = 5;
}

void VoiceDialogWnd::timerEvent(QTimerEvent * ev)
{
    if(ev->timerId() == timerId) //通话时长
    {
        timeLenght += 1;
        ui->label->setText(QString("正在通话 %1:%2:%3").arg(timeLenght / 3600).arg(timeLenght % 3600 / 60).arg(timeLenght % 60));
    }
    if(ev->timerId() == timerId2) //呼叫时长
    {
        //超时
        killTimer(timerId2);
        emit closeBtn(timeLenght);
    }
}

void VoiceDialogWnd::on_agreeBtn_clicked()
{
    //同意
    emit agreeBtn();
}

void VoiceDialogWnd::on_disAgreeBtn_clicked()
{
    //拒绝
    emit disAgreeBtn();
    this->close();
}

void VoiceDialogWnd::on_closeBtn_clicked()
{
    //挂断/取消通话
    killTimer(timerId2);
    emit closeBtn(timeLenght);
}

void VoiceDialogWnd::on_closeBtn_2_clicked()
{
    if(state == 1 || state == 3)
    {
        killTimer(timerId2);
        emit closeBtn(timeLenght);
    }
    else if(state == 2)
    {
        emit disAgreeBtn();
    }
    this->close();
}
