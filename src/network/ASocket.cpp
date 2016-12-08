/*!
* \file ASocket.cpp
* \author Nathanael Landais
*/

#include	<iostream>
#include	"ASocket.h"
#include  "../utils/Log.h"

namespace Yo
{
  namespace Network
  {
    ASocket::ASocket() { m_open = false; }

    ASocket::~ASocket()
    {
      if (m_open)
        this->Close();
    }

    void ASocket::Close()
    {
      m_lock.lock();
      if (m_open)
      {
        closesocket(m_sock);
        m_open = false;
        Utils::Log::writeInfo(GetIP_intern() + ": Socket's connection ended");
      }
      m_lock.unlock();
    }

    void ASocket::init()
    {
#if defined(WIN32) || defined(_WIN32)
      WSADATA		wsa;
      int			error;

      if ((error = WSAStartup(MAKEWORD(2, 2), &wsa)) < 0)
      {
  	    Utils::Log::writeError("Socket::init(): WSAStartup failed");
        exit(EXIT_FAILURE);
      }
#endif
    }

    void ASocket::end()
    {
#if defined(WIN32) || defined(_WIN32)
      WSACleanup();
#endif
    }

    SOCKET ASocket::GetSock() const
    {
      return (m_sock);
    }

    const std::string ASocket::GetIP()
    {
      std::unique_lock<std::mutex> l(m_lock);
      return (GetIP_intern());
    }

    const std::string ASocket::GetIP_intern()
    {
      return (inet_ntoa(m_sin.sin_addr));
    }
  }
}
