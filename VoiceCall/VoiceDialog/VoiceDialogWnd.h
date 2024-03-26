#pragma once
#ifndef VOICEDIALOGWND_H
#define VOICEDIALOGWND_H

#include <QWidget>

namespace Ui {
class VoiceDialogWnd;
}

class VoiceDialogWnd : public QWidget
{
    Q_OBJECT

public:
    explicit VoiceDialogWnd(QWidget *parent = nullptr);
    ~VoiceDialogWnd();
void paintEvent(QPaintEvent *event);
    void NewCall(QString name);
    void WaitReply();
    void StartChat();
    void DisAgree();
    void ChatEnd();
    void TimeOut();
    int State(){ return state; }

private:
    void timerEvent(QTimerEvent * ev);

private slots:
    void on_agreeBtn_clicked();
    void on_disAgreeBtn_clicked();
    void on_closeBtn_clicked();

    void on_closeBtn_2_clicked();

signals:
    void agreeBtn();
    void disAgreeBtn();
    void closeBtn(int);

private:
    Ui::VoiceDialogWnd *ui;
    int timerId = 0;
    int timeLenght = 0;
    int timerId2 = 0;

    int state = 0;
};

#endif // VOICEDIALOGWND_H
