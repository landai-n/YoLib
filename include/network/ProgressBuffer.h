/*!
* \file ProgressBuffer.h
* \author Nathanael Landais
*/

#ifndef YO_PROGRESSBUFFEER_H
#define YO_PROGRESSBUFFEER_H

# include	"ASocket.h"
# include	"Select.h"

namespace Yo
{
  namespace Network
  {
    class ProgressBuffer
    {
    public:
      enum State
      {
        Handshake,
        Header,
        Payload
      };
      uint32_t           Read;
      ByteBuffer::Header PacketHeader;

      ProgressBuffer();
      inline char *GetBuffer() const { return (Buffer); }
      inline uint32_t GetSize() const { return (Size); }
      inline State GetState() const { return (PState); }
      void SetSize(uint32_t p_size);
      void SetState(State p_state);
      void Reset();

    private:
      char     *Buffer;
      uint32_t Size;
      State    PState;

      void ResetIntern();
    };
  }
}
#endif
