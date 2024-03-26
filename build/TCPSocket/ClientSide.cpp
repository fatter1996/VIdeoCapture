#include "ClientSide.h"
#include <QSettings>
#include <QDebug>
#include "Capture/Capture.h"
#include "Universal.h"

ClientSide::ClientSide(QObject* parent)
{
    m_pSocket = new QTcpSocket;
    connect(m_pSocket, &QTcpSocket::connected, this, &ClientSide::onConnected);
    connect(m_pSocket, &QTcpSocket::readyRead, this, &ClientSide::onReceive);
    timerId = startTimer(1000);
    timerId2 = startTimer(1000);
}

ClientSide::~ClientSide()
{
    killTimer(timerId);
    if(m_pSocket != nullptr)
    {
        m_pSocket->disconnectFromHost();//断开与服务器的连接
        m_pSocket->close();
        m_pSocket = nullptr;
    }
}

void ClientSide::onConnected()
{
    qDebug() << QString("TCPServer connection succeeded: %1:%2").arg(GlobalData::m_pConfigInfo->serverIp).arg(GlobalData::m_pConfigInfo->serverPORT);
    //连接成功后发送rtmp推流地址
    QByteArray array;
    array.append(QByteArray::fromHex("efefefef"));
    array.append(GlobalData::getLittleEndian_2Bit(GlobalData::m_pConfigInfo->outUrl.length() + 18));
    array.append(QByteArray::fromHex("0000"));
    array.append(QByteArray::fromHex("ff"));
    array.append(QByteArray::fromHex("80"));
    array.append(GlobalData::getLittleEndian_2Bit(GlobalData::m_pConfigInfo->outUrl.length()));
    array.append(GlobalData::m_pConfigInfo->outUrl.toLatin1());
    array.append(GlobalData::getLittleEndian_2Bit(GlobalData::m_pConfigInfo->stationName.length()));
    array.append(GlobalData::m_pConfigInfo->stationName.toLatin1());
    array.append(QByteArray::fromHex("fefefefe"));
    m_pSocket->write(array);
    flag = 0;
}

void ClientSide::onReceive()
{
    QByteArray receiveStr = m_pSocket->readAll();
    if(receiveStr.length() == 15) //心跳信息
    {
        flag = 0;
    }
    else if(receiveStr.length() == 16) //连接成功回执
    {
        timerId2 = startTimer(1000);
    }
}

void ClientSide::HeartBeat()
{
    QByteArray heart;
    heart.append(QByteArray::fromHex("efefefef"));
    heart.append(QByteArray::fromHex("0f00"));
    heart.append(QByteArray::fromHex("0000"));
    heart.append(QByteArray::fromHex("ff"));
    heart.append(QByteArray::fromHex("00"));
    heart.append(QByteArray::fromHex("5b"));
    heart.append(QByteArray::fromHex("fefefefe"));
    m_pSocket->write(heart);
}

void ClientSide::timerEvent(QTimerEvent * ev)
{
    if(ev->timerId() == timerId)
    {
        //killTimer(timerId);
        if(flag >= 3)
        {
            m_pSocket->disconnectFromHost();
            m_pSocket->connectToHost(GlobalData::m_pConfigInfo->serverIp, GlobalData::m_pConfigInfo->serverPORT);
        }
        flag++;
    }
    if(ev->timerId() == timerId2)
    {
        HeartBeat();
    }
}


