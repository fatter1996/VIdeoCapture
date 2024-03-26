#pragma once
#ifndef VOICEDIALOG_H
#define VOICEDIALOG_H

#include <QThread>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QAudioOutput>
#include "VoiceCall/UDPSocket/UdpSocket.h"
#include "VoiceCall/TCPSocket/TcpSocket.h"
#include "VoiceDialogWnd.h"

class VoiceDialog : public QThread
{
    Q_OBJECT
public:
    VoiceDialog(QObject *parent = nullptr);
    ~VoiceDialog();

    void RequestCall();
    void setAddress(QString ip, uint port);

private:
    void onRequestCall(QByteArray data);
    void onRequestReply(QByteArray data);
    void onCalling(uint type);
    void onHangUp(QByteArray data);
    void onCallEnd(QByteArray data);
    void onCallTimeOut();
    QByteArray HeartBeat(uint type);

    void getVoiceData();

public slots:
    void onResponse(QByteArray data);
    void onVoiceReceive(QByteArray);
    void onSignalling(QByteArray);
    void onAgreeBtn();
    void onDisAgreeBtn();
    void onCloseBtn(int);
    void onNewConnection(QString, uint, QString);
    void onDisConnection(QString, uint);
    void onCall(QString, uint);


signals:
    void NewMember(QString, uint, QString);
    void MemberQuit(QString, uint);

private:
    QAudioFormat m_Format; //音频参数配置
    QAudioInput* m_pAudioInput = nullptr; //音频输入
    QAudioOutput* m_pAudioOutput = nullptr; //音频输出
    QIODevice* m_pInputDevice = nullptr; //音频输入设备
    QIODevice* m_pOutputDevice = nullptr; //音频输出设备

    CallTcpSocket* m_pTcpSocket = nullptr;
    UdpSocket* m_pUdpSocket = nullptr;

    QString receiveIp;
    uint receivePORT = 0;
    bool isRunning = false;
    VoiceDialogWnd* m_pDialogWnd = nullptr;
};

#endif // VOICEDIALOG_H
