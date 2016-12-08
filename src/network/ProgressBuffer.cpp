/*!
* \file ProgressBuffer.cpp
* \author Nathanael Landais
*/

#include "ProgressBuffer.h"

namespace Yo
{
  namespace Network
  {
    ProgressBuffer::ProgressBuffer()
    {
      Size = 0;
      Read = 0;
      PState = Handshake;
      Buffer = nullptr;
    }

    void ProgressBuffer::SetSize(uint32_t p_size)
    {
      if (Size < p_size)
      {
        char *l_buffer = Buffer;

        Buffer = new char[p_size];
        if (l_buffer != nullptr)
        {
          memcpy(Buffer, l_buffer, Size);
          delete[] (l_buffer);
        }
        Size = p_size;
      }
    }

    void ProgressBuffer::SetState(State p_state)
    {
      if (p_state != PState)
      {
        ResetIntern();
        this->PState = p_state;
      }
    }

    void ProgressBuffer::Reset()
    {
      ResetIntern();
      PState = Header;
      PacketHeader = ByteBuffer::Header();
    }

    void ProgressBuffer::ResetIntern()
    {
      Size = 0;
      Read = 0;
      if (Buffer != nullptr)
        delete[] (Buffer);
      Buffer = nullptr;
    }
  }
}