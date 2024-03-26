#pragma once
#ifndef CUDPSOCKET_H
#define CUDPSOCKET_H

#include <QCoreApplication>
#include <QUdpSocket>
#include <QTimer>
#include <QThread>
class UdpSocket : public QThread
{
    Q_OBJECT

public:
    UdpSocket(QString ip, uint port, QIODevice* outputDevice);
    ~UdpSocket();

    qint64 send(QByteArray msg);
    void heartBeat();
    void stop();

protected:
    virtual void run() override;
    
signals:
    void voiceReceive(QByteArray);

private:
    QUdpSocket *_pSocket = NULL;
    bool isRunning = false;
    QString receiveIp;
    uint receivePORT = 0;
    QIODevice* _outputDevice = nullptr;
};

#endif // CUDPSOCKET_H
