/*!œ
* \file TCPClientSSL.cpp
* \author Nathanael Landais
*/

#include	"Log.h"
#include  "TCPClientSSL.h"
#include 	"Dev.h"
#include  "ByteBuffer.h"

namespace Yo
{
  using namespace Utils;

  namespace Network
  {
    TCPClientSSL::TCPClientSSL(SSL *p_sslSock, SOCKET p_sock, SOCKADDR_IN p_sin)
        : TCPClient(p_sock, p_sin)
    {
      m_sockSSL = p_sslSock;
    }

    ssize_t TCPClientSSL::BaseRecv(char *p_buffer, ssize_t p_size)
    {
      return SSL_read(m_sockSSL, p_buffer, (int)p_size);
    }

    ssize_t TCPClientSSL::BaseSend(const char *p_buffer, ssize_t p_size)
    {
      return SSL_write(m_sockSSL, p_buffer, (int)p_size);
    }

    int TCPClientSSL::Receive(ByteBuffer **p_packet)
    {
      int ret;

      while ((ret = TCPClient::Receive(p_packet)) == RECV_PROGRESS && SSL_pending(m_sockSSL));
      return (ret);
    }
  }
}
