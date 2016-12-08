/*!
* \file WebSocket.h
* \author Nathanael Landais
*/

#ifndef YO_WEBSOCKET_H
#define YO_WEBSOCKET_H

# include <string>
# include "Protocol.h"

# define WS_KEY "Sec-WebSocket-Key"
# define WS_ACCEPT_KEY "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
# define WS_HOST "Host"

namespace Yo
{
  namespace Network
  {
    class TCPClient;
    class WebSocket : public Protocol
    {
    public:
      union u_mask
      {
        uint32_t integer;
        uint8_t tab[4];
      };
      WebSocket(TCPClient *);
      virtual         ~WebSocket() {}
      virtual int    Handshake(ProgressBuffer *);
      bool            ReadHeader();
      char            *CreateHeader(ssize_t, ssize_t &);
      void            DecodeData(char *, uint32_t);
      void            EncodeData(char *, uint32_t, u_mask);
      virtual ssize_t Receive(char *, ssize_t);
      virtual ssize_t Send(char *, ssize_t);
    private:
      struct s_webSocketPacketData
      {
        bool          isMasked;
        union u_mask  mask;
        uint64_t      length;
        uint64_t      pos;
        uint32_t      opcode;
        bool          lastFrame;
      };
      virtual bool HandshakeAnswer();
      bool IsEndOfFrame(const char *, int);

      std::string                  m_key;
      std::string                  m_host;
      struct s_webSocketPacketData m_packet;
    };
  }
}

#endif
