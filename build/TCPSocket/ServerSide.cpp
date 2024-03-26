#include "ServerSide.h"
#include <QSettings>
#include <QDebug>
#include "Universal.h"
#pragma execution_character_set("utf-8")

ServerSide::ServerSide(QObject* parent)
{
    m_pServer = new QTcpServer;

    m_pServer->listen(QHostAddress(GlobalData::m_pConfigInfo->serverIp), GlobalData::m_pConfigInfo->serverPORT); //绑定本地IP和端口
    connect(m_pServer, &QTcpServer::newConnection, this, &ServerSide::onConnection);
}

ServerSide::~ServerSide()
{
    if(m_pServer)
    {
        m_pServer->close();
        m_pServer = nullptr;
    }
}

void ServerSide::onConnection()
{
    //取出连接好的套接字
    QTcpSocket* m_pSocket = m_pServer->nextPendingConnection();

    //获得通信套接字的控制信息
    QString ip = m_pSocket->peerAddress().toString();//获取连接的 ip地址
    quint16 port = m_pSocket->peerPort();//获取连接的 端口号
    connect(m_pSocket, &QTcpSocket::readyRead, this, [=](){
        qDebug() << "--- Receive ---";
        QByteArray receiveStr = m_pSocket->readAll();
        qDebug() << receiveStr.toHex();
        if(receiveStr.length() == 0)
            return;
        if(receiveStr.length() == 15) //心跳包
        {
            QByteArray heart;
            heart.append(QByteArray::fromHex("efefefef"));
            heart.append(QByteArray::fromHex("0f00"));
            heart.append(QByteArray::fromHex("0000"));
            heart.append(QByteArray::fromHex("ee"));
            heart.append(QByteArray::fromHex("00"));
            heart.append(QByteArray::fromHex("5b"));
            heart.append(QByteArray::fromHex("fefefefe"));

            qDebug() << "--- Sender ---";
            m_pSocket->write(heart);
            qDebug() << heart.toHex();
        }
        else emit Receive(receiveStr);
    });
    m_pSocketList.append(m_pSocket);
    qDebug() << QString("客户端连接成功: %1:%2").arg(ip).arg(port);
}

void ServerSide::send(QByteArray msg)
{
    qDebug() << "--- Sender ---";
    //m_pSocket->write(msg);
    qDebug() << msg.toHex();
}
