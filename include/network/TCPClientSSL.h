/*!
* \file TCPClientSSL.h
* \author Nathanael Landais
*/

#ifndef   YO_TCPClientSSL_H
# define  YO_TCPClientSSL_H

# include	"TCPClient.h"

namespace Yo
{
  namespace Network
  {
    class TCPClientSSL : public TCPClient
    {
    public:
      TCPClientSSL(SSL *, SOCKET, SOCKADDR_IN);
      virtual ~TCPClientSSL(){}

    private:
      virtual ssize_t BaseRecv(char *, ssize_t);
      virtual ssize_t BaseSend(const char *, ssize_t);
      virtual int Receive(ByteBuffer **);

      SSL         *m_sockSSL;
    };
  }
}
#endif
