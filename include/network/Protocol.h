/*!
* \file Protocol.h
* \author Nathanael Landais
*/

#ifndef YO_PROTOCOL_H
#define YO_PROTOCOL_H

#include <string>
#include <cstring>
#include "../utils/Log.h"
#include "ProgressBuffer.h"

#define 	KB_TO_BYTES(x) x * 1024
#define 	MB_TO_BYTES(x) KB_TO_BYTES(x) * 1024
#define 	BYTES_TO_KB(x) x / 1024
#define 	BYTES_TO_MB(x) BYTES_TO_KB(x) /1024
#define		PACKET_SIZE_LIMIT 30

# define RECV_SUCCESS 0
# define RECV_HANDSHAKE 1
# define RECV_PROGRESS 2
# define RECV_ERROR 3

namespace Yo
{
  namespace Network
  {
    class TCPClient;
    class Protocol
    {
    public:
      enum Type
      {
        Raw,
        WebSocket,
        Unknown
      };

      bool operator==(const Type &p_type) { return (m_type == p_type); }
      virtual ~Protocol() {}
      virtual int Handshake(ProgressBuffer *) = 0;
      virtual ssize_t Receive(char *, ssize_t ) = 0;
      virtual ssize_t Send(char *, ssize_t) = 0;
      virtual ssize_t ConstSend(const char *p_data, ssize_t p_size)
      {
        char *newData = new char[p_size];

        memcpy(newData, p_data, p_size);
        ssize_t result = Send(newData, p_size);
        delete [](newData);
        return (result);
      }
    protected:
      Protocol(TCPClient *p_socket) : m_socket(p_socket) {}
      Type m_type;
      TCPClient *m_socket;
    };
  }
}
#endif
