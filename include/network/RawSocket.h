/*!
* \file RawSocket.h
* \author Nathanael Landais
*/

#ifndef YO_RAWSOCKET_H
#define YO_RAWSOCKET_H

# include "Protocol.h"

namespace Yo
{
  namespace Network
  {
    class TCPClient;
    class RawSocket : public Protocol
    {
    public:
      RawSocket(TCPClient *p_socket) : Protocol(p_socket) { m_type = Protocol::Raw; }
      virtual           ~RawSocket() {}
      virtual ssize_t   Receive(char *, ssize_t);
      virtual ssize_t   Send(char *, ssize_t);
      virtual int      Handshake(ProgressBuffer *);
    };
  }
}

#endif
