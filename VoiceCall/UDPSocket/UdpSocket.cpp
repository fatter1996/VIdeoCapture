#include "UdpSocket.h"
#include <QSettings>
#include "Universal.h"
#include <QDateTime>

UdpSocket::UdpSocket(QString ip, uint port, QIODevice* outputDevice) : receiveIp(ip), receivePORT(port), _outputDevice(outputDevice)
{
    _pSocket = new QUdpSocket;
    _pSocket->bind(QHostAddress(GlobalData::m_pCallConfigInfo->localIp), GlobalData::m_pCallConfigInfo->localPORT);  //绑定本地IP和端口
    _pSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 1024 * 5);

    //启动定时器
    //timerId = startTimer(1000);
}

UdpSocket::~UdpSocket()
{
    delete _pSocket;
}

qint64 UdpSocket::send(QByteArray msg) //消息发送
{
    qDebug() << "--- UdpSocket Send Voice ---";
    qDebug() << QString("%1:%2  %3").arg(receiveIp).arg(receivePORT).arg(msg.size());
    if(!_pSocket) return 0;
    return _pSocket->writeDatagram(msg, QHostAddress(receiveIp), receivePORT);
}

void UdpSocket::stop()
{
    isRunning = false;
    _pSocket->close();
}

void UdpSocket::run()
{
    isRunning = true;
    while(isRunning)
    {
        msleep(1);
        QByteArray receiveStr;
        while(_pSocket->hasPendingDatagrams())
        {
            receiveStr.resize(_pSocket->pendingDatagramSize());
            _pSocket->readDatagram(receiveStr.data(), receiveStr.size());
        }

        if(receiveStr.length() > 0)
        {
            _outputDevice->write(receiveStr);
            //emit voiceReceive(receiveStr);
        }   
    }
}
