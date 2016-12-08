/*!
* \file RawSocket.cpp
* \author Nathanael Landais
*/

#include "RawSocket.h"
#include "TCPClient.h"

namespace Yo
{
  namespace Network
  {
    ssize_t RawSocket::Receive(char *p_buffer, ssize_t p_size)
    {
      return (m_socket->BaseRecv(p_buffer, p_size));
    }

    ssize_t RawSocket::Send(char *p_buffer, ssize_t p_size)
    {
      return (m_socket->BaseSend(p_buffer, p_size));
    }

    int RawSocket::Handshake(ProgressBuffer *)
    {
      std::string answer = "READY";

      return ((m_socket->BaseSend(answer.c_str(), answer.size()) > 0) ?
       RECV_SUCCESS : RECV_ERROR);
    }
  }
}
