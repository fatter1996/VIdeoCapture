#ifndef SERVERSIDE_H
#define SERVERSIDE_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class ServerSide : public QObject
{
     Q_OBJECT
public:
    ServerSide(QObject* parent = nullptr);
    ~ServerSide();

    void send(QByteArray msg);
public slots:
    void onConnection();

signals:
    void Receive(QByteArray);

private:
    QTcpServer* m_pServer = nullptr;

    QVector<QTcpSocket*> m_pSocketList;
};

#endif // SERVERSIDE_H
