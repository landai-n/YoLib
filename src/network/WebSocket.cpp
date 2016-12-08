/*!
* \file WebSocket.cpp
* \author Nathanael Landais
*/

#include <random>
#include "WebSocket.h"
#include "StringParser.h"
#include "Hash.h"
#include "TCPClient.h"
#include "Log.h"
#include "Binary.h"

#define GET_MASK_LOW_PART(pos, size) (0xFF >> (8 - (pos + size))) << (8 - (pos + size))
#define GET_PARTIAL_BYTE(value, pos, size) (value & GetMask(pos, size))

namespace Yo
{
  using namespace Utils;

  namespace Network
  {
    WebSocket::WebSocket(TCPClient *p_socket) : Protocol(p_socket)
    {
      m_packet.lastFrame = true;
      m_type = Protocol::WebSocket;
      m_packet.mask.integer = 0;
      m_packet.length = 0;
      m_packet.pos = 0;
      m_packet.opcode = 0;
    }

    ssize_t WebSocket::Receive(char *p_buffer, ssize_t p_size)
    {
      if (m_packet.pos >= m_packet.length)
        ReadHeader();
      ssize_t ret = m_socket->BaseRecv(p_buffer, p_size);

      if (ret >= 0)
      {
        m_packet.pos += ret;
        DecodeData(p_buffer, ret);
      }
      return (ret);
    }

    ssize_t WebSocket::Send(char *p_buffer, ssize_t p_size)
    {
      ssize_t headerSize;
      ssize_t ret;
      char *headerData = CreateHeader(p_size, headerSize);
      char *finalBuffer;

      finalBuffer = new char[p_size + headerSize];
      memcpy(finalBuffer, headerData, headerSize);
      memcpy(finalBuffer + headerSize, p_buffer, p_size);
      ret = m_socket->BaseSend(finalBuffer, headerSize + p_size);
      delete[] (finalBuffer);
      delete[] (headerData);
      return (ret);
    }

    int WebSocket::Handshake(ProgressBuffer *p_buffer)
    {
      if (!IsEndOfFrame(p_buffer->GetBuffer(), p_buffer->Read))
      {
        ssize_t ret;

        p_buffer->SetSize(p_buffer->GetSize() + 1025);
        if ((ret = m_socket->BaseRecv(p_buffer->GetBuffer() + p_buffer->Read, 1024)) <= 0)
        {
          Utils::Log::writeError("WebSocket::Handshake()", m_socket->GetIP() + ": Incomplete Websocket handshake");
          return (RECV_ERROR);
        }
        p_buffer->Read += ret;
        if (!IsEndOfFrame(p_buffer->GetBuffer(), p_buffer->Read))
         return (RECV_PROGRESS);
         p_buffer->GetBuffer()[p_buffer->Read] = 0;
      }
      if ((m_key = StringParser::GetWord(StringParser::GetFirstLine(p_buffer->GetBuffer(), WS_KEY), 2)) == "")
       return (RECV_ERROR);
      if ((m_host = StringParser::GetWord(StringParser::GetFirstLine(p_buffer->GetBuffer(), WS_HOST), 2)) == "")
       return (RECV_ERROR);
      return (HandshakeAnswer() ? RECV_SUCCESS : RECV_ERROR);
     }

     bool WebSocket::IsEndOfFrame(const char *p_frame, int p_size)
     {
      if (p_size < 4)
      return (false);
      return (HandshakeAnswer());
      return (strncmp(p_frame + p_size - 4, "\r\n\r\n", 4) == 0);
     }

    bool WebSocket::HandshakeAnswer()
    {
      std::string answer;
      std::string answerKey = Hash::GenerateSha1(m_key, WS_ACCEPT_KEY, Hash::Base64);

      answer += "HTTP/1.1 101 Switching Protocols\r\n";
      answer += "Upgrade: websocket\r\n";
      answer += "Connection: Upgrade\r\n";
      answer += "Sec-WebSocket-Accept: " + answerKey + "\r\n";
      answer += "\r\n";

      return (m_socket->BaseSend(answer.c_str(), answer.size()) > 0);
    }

    uint8_t GetMask(uint32_t pos, uint32_t size)
    {
     uint8_t mask = (uint8_t)(GET_MASK_LOW_PART(pos, size) << pos);
     uint8_t finalMask = mask >> pos;

     return (finalMask);
    }

    bool WebSocket::ReadHeader()
    {
      unsigned char headerData[2];
      char *extendedHeaderData;

      if (m_socket->BaseRecv((char *)headerData, sizeof(headerData)) <= 0)
        return (false);
      int byte = 0;
      m_packet.length = 0;
      m_packet.pos = 0;
      m_packet.mask.integer = 0;

      m_packet.lastFrame = GET_PARTIAL_BYTE(headerData[0], 0, 1);
      m_packet.opcode = GET_PARTIAL_BYTE(headerData[0], 4, 4);
      m_packet.isMasked = GET_PARTIAL_BYTE(headerData[1], 0, 1);
      m_packet.length = GET_PARTIAL_BYTE(headerData[1], 1, 7);
      int extendedHeaderSize = m_packet.isMasked * 4 + (m_packet.length == 126) * 2 + (m_packet.length == 127) * 8;
      extendedHeaderData = new char[extendedHeaderSize];

      if (m_socket->BaseRecv(extendedHeaderData, extendedHeaderSize) <= 0)
        return (false);
      byte = 0;
      if (m_packet.length == 126)
      {
        m_packet.length = 0;
        memcpy(&m_packet.length, extendedHeaderData + byte, 2);
        byte += 2;
      }
      else if (m_packet.length == 127)
      {
        m_packet.length = 0;
        memcpy(&m_packet.length, extendedHeaderData + byte, 8);
        byte += 8;
      }
      if (m_packet.isMasked)
        memcpy(&(m_packet.mask.integer), extendedHeaderData + byte, 4);
      return (true);
    }

    char *WebSocket::CreateHeader(ssize_t p_size, ssize_t &p_headerSize)
    {
      char *header;
      int byte = 2;
      p_headerSize = 2;
      if (p_size > 126)
        p_headerSize += (p_size < std::numeric_limits<uint16_t>::max()) ? 2 : 8;
      header = new char[p_headerSize]();
      header[0] = B8(10000010);
      header[1] = B8(00000000) | uint8_t((p_headerSize == 2) ? p_size : (p_headerSize == 4) ? 126 : 127);
      if (p_headerSize == 4)
      {
        memcpy(header + byte, &p_size, 2);
        byte += 2;
      }
      else if (p_headerSize == 10)
      {
        memcpy(header + byte, &p_size, 8);
        byte += 8;
      }
      return (header);
    }

    void WebSocket::DecodeData(char *p_data, uint32_t p_size)
    {
      for (uint32_t i = 0; i < p_size; ++i)
        p_data[i] = p_data[i] ^ m_packet.mask.tab[i % 4];
    }

    void WebSocket::EncodeData(char *p_data, uint32_t p_size, u_mask p_mask)
    {
      for (uint32_t i = 0; i < p_size; ++i)
        p_data[i] = p_data[i] ^ p_mask.tab[i % 4];
    }
  }
}
