#pragma once
#ifndef SERVERSIDE_H
#define SERVERSIDE_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>


class CallServerSide : public QObject
{
     Q_OBJECT
public:
    CallServerSide(QObject* parent = nullptr);
    ~CallServerSide();

    void sendToClient(QByteArray data, QString ip, uint port);

private:
    QByteArray HeartBeat();
    void timerEvent(QTimerEvent * ev) override;

public slots:
    void onConnection();

signals:
    void Signalling(QByteArray);
    void NewConnection(QString, uint, QString);
    void DisConnection(QString, uint);

private:
    QTcpServer* m_pServer = nullptr;
    QTcpSocket* m_pCurrSocket = nullptr;
    QVector<QTcpSocket*> m_pSocketList;
};

#endif // SERVERSIDE_H
