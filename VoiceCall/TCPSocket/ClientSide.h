#ifndef CLIENTSIDE_H
#define CLIENTSIDE_H

#include <QObject>
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QFile>

class CallClientSide : public QObject
{
     Q_OBJECT
public:
    CallClientSide(QObject* parent = nullptr);
    ~CallClientSide();

    void sendToServer(QByteArray data);

private:
    void HeartBeat();
    void timerEvent(QTimerEvent * ev);

public slots:
    void onConnected();
    void onReceive();

signals:
    void Receive(QByteArray);
    void Signalling(QByteArray);

private:
    QTcpSocket* m_pSocket = nullptr;

    int flag = 0;
    int timerId = -1; //超时计时器
    int timerId2 = -1; //心跳计时器
};

#endif // CLIENTSIDE_H
