/*!
* \file TCPListener.h
* \author Nathanael Landais
*/

#ifndef YO_TCPListener_H
#define YO_TCPListener_H

# include	"ASocket.h"

namespace Yo
{
  namespace Network
  {
    class TCPClient;
    class TCPListener : public ASocket
    {
    public:
      virtual bool		  Open(uint32_t, uint16_t);
      virtual TCPClient *WaitNewSocket();
    };
  }
}
#endif
