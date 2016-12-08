/*!
* \file Socket.h
* \author Nathanael Landais
*/

#ifndef		YO_ASOCKET_H
# define	YO_ASOCKET_H

# include	<mutex>
# include	<cstdint>
# include	"ByteBuffer.h"

#if defined(WIN32) || defined(_WIN32)

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>

typedef int socklen_t;

#elif defined(linux) || defined(__unix) || defined(__linux)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#endif

# include <openssl/bio.h>
# include <openssl/ssl.h>
# include <openssl/err.h>

namespace Yo
{
  namespace Network
  {
    /**
     * @brief ASocket base class
     */
    class ASocket
    {
    public:
      ASocket();
      virtual       ~ASocket();
      virtual void  Close();
      virtual bool  IsOpen() { return (m_open); }
      static void		init();
      static void		end();
      SOCKET			  GetSock() const;
      const std::string GetIP();
    protected:
      const std::string GetIP_intern();

      SOCKET			  m_sock;
      SOCKADDR_IN		m_sin;
      std::mutex		m_lock;
      bool			    m_open;
    };
  }
}
#endif
