#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include "ClientSide.h"
#include "ServerSide.h"

class CallTcpSocket : public CallServerSide, public CallClientSide
{

};

#endif // TCPSOCKET_H
