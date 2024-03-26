#include "ClientSide.h"
#include <QSettings>
#include <QDebug>
#include "Universal.h"

CallClientSide::CallClientSide(QObject* parent)
{
    m_pSocket = new QTcpSocket;
    connect(m_pSocket, &QTcpSocket::connected, this, &CallClientSide::onConnected);
    connect(m_pSocket, &QTcpSocket::readyRead, this, &CallClientSide::onReceive);
    timerId = startTimer(1000);
    timerId2 = startTimer(1000);
}

CallClientSide::~CallClientSide()
{
    killTimer(timerId);
    if(m_pSocket != nullptr)
    {
        m_pSocket->disconnectFromHost();//断开与服务器的连接
        m_pSocket->close();
        m_pSocket = nullptr;
    }
}

void CallClientSide::onConnected()
{
    //连接成功后发送学员名称
    QByteArray array;
    array.append(QByteArray::fromHex("efefefef"));
    array.append(GlobalData::getLittleEndian_2Bit(GlobalData::m_pCallConfigInfo->name.toLocal8Bit().length() + 16));
    array.append(QByteArray::fromHex("0000"));
    array.append(QByteArray::fromHex("ff"));
    array.append(QByteArray::fromHex("96"));
    array.append(GlobalData::getLittleEndian_2Bit(GlobalData::m_pCallConfigInfo->name.toLocal8Bit().length()));
    array.append(GlobalData::m_pCallConfigInfo->name.toLocal8Bit());
    array.append(QByteArray::fromHex("fefefefe"));
    sendToServer(array);
    flag = 0;
}

void CallClientSide::onReceive()
{
    QByteArray receiveStr = m_pSocket->readAll();
    if(receiveStr.length() <= 10)
        return;
    //长度校验
    if(receiveStr.length() != GlobalData::getBigEndian_2Bit(receiveStr.mid(4,2)))
        return;

    if((unsigned char)receiveStr.at(9) == 0x00 && receiveStr.length() == 14)
    {
        flag = 0;
    }
    else emit Signalling(receiveStr);
    qDebug() << "----TCP Receive----";
    qDebug() << receiveStr.toHex();
}

void CallClientSide::sendToServer(QByteArray data)
{
    m_pSocket->write(data);
    qDebug() << "----TCP Send----";
    qDebug() << data.toHex();
}

void CallClientSide::HeartBeat()
{
    QByteArray heart;
    heart.append(QByteArray::fromHex("efefefef"));
    heart.append(QByteArray::fromHex("0e00"));
    heart.append(QByteArray::fromHex("0000"));
    heart.append(QByteArray::fromHex("ff"));
    heart.append(QByteArray::fromHex("00"));
    heart.append(QByteArray::fromHex("fefefefe"));
    m_pSocket->write(heart);
}

void CallClientSide::timerEvent(QTimerEvent * ev)
{
    if(ev->timerId() == timerId)
    {
        if(flag >= 3)
        {
            m_pSocket->disconnectFromHost();

            if(m_pSocket->bind(QHostAddress(GlobalData::m_pCallConfigInfo->localIp_Tcp), GlobalData::m_pCallConfigInfo->localPORT_Tcp))
            {
                qDebug() << "1 LocalHost:" << m_pSocket->localAddress();
                qDebug() << "2 localPort:" << m_pSocket->localPort();
            }
            m_pSocket->connectToHost(GlobalData::m_pCallConfigInfo->serverIp, GlobalData::m_pCallConfigInfo->serverPORT);
        }
        flag++;
    }
    if(ev->timerId() == timerId2)
    {
        HeartBeat();
    }
}


