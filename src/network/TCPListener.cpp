/*!
* \file TCPListener.cpp
* \author Nathanael Landais
*/

#include	"Log.h"
#include  "TCPListener.h"
#include  "TCPClient.h"
#include  "ConfigManager.h"

namespace Yo
{
  using namespace Utils;

  namespace Network
  {
    bool TCPListener::Open(uint32_t p_port, uint16_t p_queueSize)
    {
      if ((m_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
      {
        Utils::Log::writeError("TCPListener::Open()");
        return (false);
      }
      memset(&m_sin, 0, sizeof(m_sin));
      m_sin.sin_addr.s_addr = htonl(INADDR_ANY);
      m_sin.sin_family = AF_INET;
      m_sin.sin_port = htons(p_port);
      int yes=1;
      setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
      if (bind(m_sock, (SOCKADDR *)&m_sin, sizeof(m_sin)) == SOCKET_ERROR)
      {
        Utils::Log::writeError("TCPListener::Open()");
        return (false);
      }
      if (listen(m_sock, p_queueSize) == SOCKET_ERROR)
      {
        Utils::Log::writeError("TCPListener::Open()");
        return (false);
      }
      m_open = true;
      return (true);
    }

    TCPClient *TCPListener::WaitNewSocket()
    {
      SOCKET      newSock;
      SOCKADDR_IN newSin;
      int         newSinSize = sizeof(newSin);
      if ((newSock = accept(m_sock, (SOCKADDR *) &newSin, (socklen_t *) &newSinSize)) == INVALID_SOCKET)
      {
        Utils::Log::writeError("TCPListener::WaitNewSocket()");
        return (nullptr);
      }
      Log::writeInfo("Receive association request");
      return (new TCPClient(newSock, newSin));
    }
  }
}
