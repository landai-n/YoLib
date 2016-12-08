/*!
* \file TCPClient.h
* \author Nathanael Landais
*/

#ifndef YO_TCPClient_H
#define YO_TCPClient_H

# include "ASocket.h"
# include "Protocol.h"

namespace Yo
{
  namespace Network
  {
    class ProgressBuffer;
    class TCPClient : public ASocket
    {
      friend class RawSocket;
      friend class WebSocket;
    public:
      TCPClient(SOCKET, SOCKADDR_IN);
      TCPClient();
      virtual ~TCPClient();
      virtual bool  Send(ByteBuffer &);
      virtual bool  Send(std::shared_ptr<ByteBuffer::Stack>);
      void          SetTimeout(uint32_t, uint32_t);
      virtual int   Receive(ByteBuffer **p_packet);
      bool          Connect(const std::string &p_hostname, uint16_t p_port);
      operator bool() { return !(*m_protocol == Protocol::Unknown); }
    protected:
      virtual ssize_t BaseRecv(char *, ssize_t);
      virtual ssize_t BaseSend(const char *, ssize_t);
      virtual bool    BasePending(const ByteBuffer::Header&);
      virtual int     WaitHandshake();
      virtual int     InitHandshake();

      ProgressBuffer *m_progress;
      Protocol       *m_protocol;

    private:
      short ReceiveHeader(ByteBuffer::Header&);
      short ReceivePayload(const ByteBuffer::Header&, char **);

      bool           m_isHost;
    };
  }
}

#endif
