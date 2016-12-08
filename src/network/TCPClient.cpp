/*!
* \file TCPClient.cpp
* \author Nathanael Landais
*/

#include <sstream>
#include "TCPClient.h"
#include "ProgressBuffer.h"
#include "ByteBuffer.h"
#include "RawSocket.h"
#include "WebSocket.h"

namespace Yo
{
  namespace Network
  {
    TCPClient::TCPClient(SOCKET p_sock, SOCKADDR_IN p_sin)
    {
      m_progress = new ProgressBuffer();
      m_sock = p_sock;
      m_sin  = p_sin;
      m_open = true;
      m_isHost = true;
      m_protocol = nullptr;
    }

    TCPClient::TCPClient()
    {
      m_progress = new ProgressBuffer();
      m_protocol = new RawSocket(this);
      m_isHost = false;
    }

    TCPClient::~TCPClient()
    {
      delete (m_progress);
    }

    bool TCPClient::Connect(const std::string &p_hostname, uint16_t p_port)
    {
      if (m_isHost)
        return (false);
      struct hostent *info = nullptr;

      info = gethostbyname(p_hostname.c_str());
      if ((m_sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
      {
        Utils::Log::writeError("TCPClient::Connect()");
        return (false);
      }
      memset(&m_sin, 0, sizeof(m_sin));
      m_sin.sin_addr = *(IN_ADDR *) info->h_addr;
      m_sin.sin_family = AF_INET;
      m_sin.sin_port = htons(p_port);
      if (connect(m_sock, (SOCKADDR *) &m_sin, sizeof(SOCKADDR)) == SOCKET_ERROR)
      {
        Utils::Log::writeError("TCPClient::Connect()");
        return (false);
      }
      m_open = InitHandshake() == RECV_SUCCESS;
      return (m_open);
    }

    void TCPClient::SetTimeout(uint32_t p_sec, uint32_t p_usec)
    {
      struct timeval timeout;

      timeout.tv_usec = p_usec;
      timeout.tv_sec = p_sec;
      setsockopt(m_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof timeout);
    }

    int TCPClient::WaitHandshake()
    {
      ssize_t ret = 0;

      m_progress->SetSize(4);
      if ((ret = BaseRecv(m_progress->GetBuffer() + m_progress->Read, 3 - m_progress->Read)) <= 0)
      {
        Utils::Log::writeError("TCPClient::WaitHandshake()", GetIP_intern() + ": Incomplete protocol handshake");
        return (RECV_ERROR);
      }
      m_progress->Read += ret;

      if (m_progress->Read < 3)
        return (RECV_PROGRESS);

      m_progress->GetBuffer()[3] = 0;
      std::istringstream stream((std::string(m_progress->GetBuffer())));
      std::string        line;

      std::getline(stream, line);
      if (line.find("RAW") != line.npos)
        m_protocol = new RawSocket(this);
      else if (line.find("GET") != line.npos)
        m_protocol = new WebSocket(this);
      else
      {
        Utils::Log::writeError("TCPClient::WaitHandshake()", GetIP_intern() + ": Invalid protocol handshake: " + line);
        return (RECV_ERROR);
      }
      Utils::Log::writeInfo(GetIP_intern() + ": Protocol handshake complete");
      return (RECV_SUCCESS);
    }

    int TCPClient::InitHandshake()
    {
      BaseSend("RAW", 3);

      char answer[6];
      ssize_t  ret;
      if ((ret = recv(m_sock, answer, sizeof(answer) - 1, 0)) <= 0)
       return (RECV_ERROR);
      answer[ret] = 0;
      if (std::string(answer) != "READY")
      {
        Utils::Log::writeError("TCPClient::InitHandshake", GetIP_intern() + ": Protocol handshake failed");
      return (RECV_ERROR);
      }
      m_progress->Reset();
      return (RECV_SUCCESS);
    }

    short TCPClient::ReceiveHeader(ByteBuffer::Header &p_header)
    {
      m_progress->SetSize(sizeof(ByteBuffer::Header));
      ssize_t ret = 0;

      ret = m_protocol->Receive(m_progress->GetBuffer() + m_progress->Read, sizeof(ByteBuffer::Header) - m_progress->Read);
      if (ret <= 0)
        return (RECV_ERROR);
      m_progress->Read += ret;
      if (m_progress->Read < sizeof(ByteBuffer::Header))
        return (RECV_PROGRESS);

      memcpy(&p_header, m_progress->GetBuffer(), sizeof(ByteBuffer::Header));
      if ((uint32_t) p_header.payloadSize > MB_TO_BYTES(PACKET_SIZE_LIMIT))
      {
        Utils::Log::writeError("TCPClient::ReceiveHeader()",
                               GetIP_intern() + ": Packet indicate a size of " + std::to_string(BYTES_TO_MB(p_header.payloadSize)) +
                               " MB (maximum allowed size is " + std::to_string(PACKET_SIZE_LIMIT) + " MB)");
        return (RECV_ERROR);
      }
      m_progress->SetState(ProgressBuffer::Payload);
      return (RECV_SUCCESS);
    }

    short TCPClient::ReceivePayload(const ByteBuffer::Header &p_header, char **p_payload)
    {
      m_progress->SetSize((uint32_t)p_header.payloadSize);

      ssize_t ret;
      if (BasePending(p_header))
      {
        if ((ret = m_protocol->Receive(m_progress->GetBuffer() + m_progress->Read, p_header.payloadSize - m_progress->Read)) <  0)
        {
          if (ret < 0)
            Utils::Log::writeInfo("TCPClient::ReceivePayload(): " + GetIP_intern() + ": " + std::string(strerror(errno)));
          return (RECV_ERROR);
        }
        else if (ret == 0)
        {
          Utils::Log::writeError("TCPClient::ReceivePayload()",
                                 GetIP_intern() + ": Invalid data format, expected " + std::to_string(p_header.payloadSize)
                                 + " bytes and getting " + std::to_string(m_progress->Read + ret) + " bytes.");
          return (RECV_ERROR);
        }
        m_progress->Read += ret;
      }
      if (m_progress->Read < (uint32_t)p_header.payloadSize)
        return (RECV_PROGRESS);
      *p_payload = m_progress->GetBuffer();
      return (RECV_SUCCESS);
    }

    int TCPClient::Receive(ByteBuffer **p_packet)
    {
      char                         *bufData;
      int                          ret;

      if (!m_open)
        return (RECV_ERROR);
      switch (m_progress->GetState())
      {
        case ProgressBuffer::Handshake:
          if (m_isHost)
          {
            if ((ret = WaitHandshake()) != RECV_SUCCESS)
            {
              if (ret == RECV_ERROR)
              {
                m_progress->Reset();
                m_progress->SetState(ProgressBuffer::Handshake);
              }
              return (ret);
            }
          }
          else
            if ((ret = InitHandshake()) != RECV_SUCCESS)
              return (ret);
          m_progress->Reset();
          return (RECV_HANDSHAKE);

        case ProgressBuffer::Header:
          ret = ReceiveHeader(m_progress->PacketHeader);
          if (ret == RECV_ERROR)
            return (RECV_ERROR);
          return (RECV_PROGRESS); // return anyway to not lock on payload read if client didn't sent it

        case ProgressBuffer::Payload:
          if ((ret = ReceivePayload(m_progress->PacketHeader, &bufData)) != RECV_SUCCESS)
            return (ret);
          *p_packet     = new ByteBuffer(bufData, m_progress->PacketHeader);
          m_progress->Reset();
          return (RECV_SUCCESS);
      }
      return (RECV_ERROR);
    }

    bool TCPClient::Send(ByteBuffer &p_data)
    {
      const char                   *buffer = p_data.GetBuffer();
      int32_t                      size    = p_data.GetSize();

      if (!m_open)
        return (false);
      if (m_protocol->ConstSend(buffer, size) <= 0)
        return (false);
      return (true);
    }

    bool TCPClient::Send(std::shared_ptr<ByteBuffer::Stack> p_packetStack)
    {
      ssize_t size = p_packetStack->GetSize();
      ssize_t readSize = 0;
      char *buffer;
      ssize_t  ret;

      if (!m_open)
        return (false);

      buffer = new char[size];
      auto packetList = p_packetStack->GetList();
      for (auto packet = packetList.begin(); packet != packetList.end(); ++packet)
      {
        if (readSize + (*packet)->GetSize() > size)
        {
          delete[] (buffer);
          return (false);
        }
        memcpy(buffer + readSize, (*packet)->GetBuffer(), (size_t)(*packet)->GetSize());
        readSize += (*packet)->GetSize();
      }
      ret = m_protocol->Send(buffer, readSize);
      delete[] buffer;
      return (ret > 0);
    }

    bool TCPClient::BasePending(const ByteBuffer::Header &p_header)
    {
      return p_header.payloadSize > 0;
    }

    ssize_t TCPClient::BaseRecv(char *p_buffer, ssize_t p_size)
    {
      return recv(m_sock, p_buffer, (size_t)p_size, 0);
    }

    ssize_t TCPClient::BaseSend(const char *p_buffer, ssize_t p_size)
    {
      return send(m_sock, p_buffer, (size_t)p_size, 0);
    }
  }
}
