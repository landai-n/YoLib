/*!
* \file TCPListenerSSL.cpp
* \author Nathanael Landais
*/

#include	"Log.h"
#include  "TCPListenerSSL.h"
#include  "TCPClientSSL.h"

namespace Yo
{
  using namespace Utils;

  namespace Network
  {
    TCPListenerSSL::TCPListenerSSL(const std::string &p_certificatePath, const std::string &p_privateKeyPath) :
      m_certificatePath(p_certificatePath), m_privateKeyPath(p_privateKeyPath) {}

    bool  TCPListenerSSL::InitSSL()
    {
      Log::writeInfo("Initialize SSL");
      SSL_load_error_strings();
      SSL_library_init();
      OpenSSL_add_all_algorithms();
      if ((m_sslctx = SSL_CTX_new(SSLv23_server_method())) == nullptr)
        return Log::writeError("TCPListenerSSL::InitSSL");
      SSL_CTX_set_options(m_sslctx, SSL_OP_SINGLE_DH_USE);
      if (SSL_CTX_use_certificate_file(m_sslctx, m_certificatePath.c_str(), SSL_FILETYPE_PEM) != 1)
        return Log::writeError("TCPListenerSSL::InitSSL", "Unable to load certificate");
      if (SSL_CTX_use_PrivateKey_file(m_sslctx, m_privateKeyPath.c_str(), SSL_FILETYPE_PEM) != 1)
        return Log::writeError("TCPListenerSSL::InitSSL", "Unable to load private key");
      return (true);
    }

    void  TCPListenerSSL::CloseSSL()
    {
      ERR_free_strings();
      EVP_cleanup();
    }

    void TCPListenerSSL::Close()
    {
      ASocket::Close();
      CloseSSL();
    }

    bool TCPListenerSSL::Open(uint32_t p_port, uint16_t p_queueSize)
    {
      if (InitSSL() == false)
        return (false);
      return (TCPListener::Open(p_port, p_queueSize));
    }

    TCPClient *TCPListenerSSL::WaitNewSocket()
    {
      SOCKET		newSock;
      SOCKADDR_IN   newSin;
      int			newSinSize = sizeof(newSin);
      struct timeval timeout;
      timeout.tv_usec = 0;


      if ((newSock = accept(m_sock, (SOCKADDR *)&newSin, (socklen_t *)&newSinSize)) == INVALID_SOCKET)
      {
        Utils::Log::writeError("TCPListenerSSL::WaitNewSocket()");
        return (nullptr);
      }
      SSL *newSSLSock = SSL_new(m_sslctx);

      timeout.tv_sec = 1; // Allow socket to wait 1s on accept before timeout
      setsockopt(newSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof timeout);
      SSL_set_fd(newSSLSock, int(newSock));
      if (SSL_accept(newSSLSock) != 1)
      {
        Log::writeError("TCPListenerSSL::WaitNewSocket", "SSL handshake failed");
        SSL_shutdown(newSSLSock);
        SSL_free(newSSLSock);
        return (nullptr);
      }
      timeout.tv_sec = 0; //Remove timeout
      setsockopt(newSock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof timeout);

      TCPClient *l_socket = new TCPClientSSL(newSSLSock, newSock, newSin);
      Log::writeInfo(l_socket->GetIP() + ": Successfull SSL handshake");
      return (l_socket);
    }
  }
}
