#include "CallButton.h"
#include <QDebug>

CallButton::CallButton(QString _ip, uint _port, QString _name) : ip(_ip), port(_port), name(_name)
{
    setFixedSize(80, 24);
    setText(name);
    setObjectName(QString("%1:%2").arg(ip).arg(port));
}

CallButton::~CallButton()
{

}

void CallButton::mousePressEvent(QMouseEvent *e)
{
    emit Call(ip, port);
}
