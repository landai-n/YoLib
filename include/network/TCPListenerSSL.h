/*!
* \file TCPListenerSSL.h
* \author Nathanael Landais
*/

#ifndef   YO_TCPListenerSSL_H
# define  YO_TCPListenerSSL_H

# include	"TCPListener.h"

namespace Yo
{
  namespace Network
  {
    class TCPClientSSL;
    class TCPListenerSSL : public TCPListener
    {
    public:
      TCPListenerSSL(const std::string &, const std::string &);
      bool                  InitSSL();
      virtual bool		      Open(uint32_t, uint16_t);
      void                  CloseSSL();
      void                  Close();
      virtual TCPClient     *WaitNewSocket();
    private:
      SSL_CTX *m_sslctx;
      const std::string m_certificatePath;
      const std::string m_privateKeyPath;
    };
  }
}
#endif
