/*!
* \file ByteBuffer.cpp
* \author Nathanael Landais
*/

#include	<exception>
#include	"ByteBuffer.h"
#include	"Zip.h"
#include	"Dev.h"
#include	"Log.h"

namespace Yo
{
  namespace Network
  {
    ByteBuffer::ByteBuffer(uint16_t p_opcode, uint16_t p_errorCode)
    {
      m_pos = sizeof(Header);
      m_header.payloadSize = 0;
      m_header.realPayloadSize = 0;
      m_header.specialId = 0;
      m_header.isCompressed = false;
      m_header.error = p_errorCode;
      m_header.opcode = p_opcode;
      m_storage.resize(GetSize());
    }

    ByteBuffer::ByteBuffer(char *p_data, Header p_header)
    {
      m_header = p_header;
      m_pos  = sizeof(Header);
      m_storage.resize(GetSize());
      WritePayload(p_data);
      if (m_header.isCompressed)
        Decompress();
      WriteHeader();
    }

    ByteBuffer::~ByteBuffer()
    {

    }

    /**
     * @brief Get packet buffer
     * @return Packet buffer
     */
    const char	*ByteBuffer::GetBuffer() const
    {
      return ((char *)(&m_storage[0]));
    }

    /**
     * @brief Get packet's size
     * @return Packet's size
     */
    int32_t ByteBuffer::GetSize() const
    {
      return (sizeof(Header) + m_header.payloadSize);
    }

    ByteBuffer	&ByteBuffer::operator<<(uint8_t p_var)
    {
      this->Append<uint8_t>(p_var);
      return (*this);
    }

    ByteBuffer	&ByteBuffer::operator<<(uint16_t p_var)
    {
      this->Append<uint16_t>(p_var);
      return (*this);
    }

    ByteBuffer	&ByteBuffer::operator<<(uint32_t p_var)
    {
      this->Append<uint32_t>(p_var);
      return (*this);
    }

    ByteBuffer	&ByteBuffer::operator<<(uint64_t p_var)
    {
      this->Append<uint64_t>(p_var);
      return (*this);
    }

    ByteBuffer	&ByteBuffer::operator<<(int8_t p_var)
    {
      this->Append<int8_t>(p_var);
      return (*this);
    }

    ByteBuffer	&ByteBuffer::operator<<(bool p_var)
    {
      this->Append<int8_t>(p_var);
      return (*this);
    }

    ByteBuffer	&ByteBuffer::operator<<(int16_t p_var)
    {
      this->Append<int16_t>(p_var);
      return (*this);
    }

    ByteBuffer	&ByteBuffer::operator<<(int32_t p_var)
    {
      this->Append<int32_t>(p_var);
      return (*this);
    }

    ByteBuffer	&ByteBuffer::operator<<(int64_t p_var)
    {
      this->Append<int64_t>(p_var);
      return (*this);
    }

    ByteBuffer	&ByteBuffer::operator<<(double p_var)
    {
      this->Append<double>(p_var);
      return (*this);
    }

    ByteBuffer	&ByteBuffer::operator<<(float p_var)
    {
      this->Append<float>(p_var);
      return (*this);
    }

    ByteBuffer &ByteBuffer::operator<<(const std::string &p_var)
    {
      for (uint32_t i(0); i < p_var.size(); ++i)
        this->Append<char>(p_var[i]);
      this->Append<char>(0);
      return (*this);
    }

    ByteBuffer &ByteBuffer::operator>>(std::string &p_var)
    {
      while (m_pos < GetSize())
      {
        char value = this->Read<uint8_t>();

        if (value == 0)
          return (*this);
        p_var += value;
      }
      Exception e(GetSize(), m_pos, "Invalid string format led to an overflow");
      Utils::Log::writeError("ByteBuffer::ReadString()", e.what());
      return (*this);
    }

    ByteBuffer &ByteBuffer::operator>>(uint8_t &p_var)
    {
      p_var = Read<uint8_t>();
      return (*this);
    }

    ByteBuffer &ByteBuffer::operator>>(uint16_t &p_var)
    {
      p_var = Read<uint16_t>();
      return (*this);
    }

    ByteBuffer &ByteBuffer::operator>>(uint32_t &p_var)
    {
      p_var = Read<uint32_t>();
      return (*this);
    }

    ByteBuffer &ByteBuffer::operator>>(uint64_t &p_var)
    {
      p_var = Read<uint64_t>();
      return (*this);
    }

    ByteBuffer &ByteBuffer::operator>>(int8_t &p_var)
    {
      p_var = Read<int8_t>();
      return (*this);
    }

    ByteBuffer &ByteBuffer::operator>>(int16_t &p_var)
    {
      p_var = Read<int16_t>();
      return (*this);
    }

    ByteBuffer &ByteBuffer::operator>>(int32_t &p_var)
    {
      p_var = Read<int32_t>();
      return (*this);
    }

    ByteBuffer &ByteBuffer::operator>>(int64_t &p_var)
    {
      p_var = Read<int64_t>();
      return (*this);
    }

    ByteBuffer &ByteBuffer::operator>>(bool &p_var)
    {
      p_var = static_cast<bool>(Read<uint8_t>());
      return (*this);
    }

    ByteBuffer &ByteBuffer::operator>>(float &p_var)
    {
      p_var = Read<float>();
      return (*this);
    }

    ByteBuffer &ByteBuffer::operator>>(double &p_var)
    {
      p_var = Read<double>();
      return (*this);
    }

    /**
     * @brief Write packet's header in buffer
     */
    void   ByteBuffer::WriteHeader()
    {
      memcpy(&m_storage[0], &m_header, sizeof(m_header));
    }

    /**
     * @brief Write packet's payload in buffer
     */
    void  ByteBuffer::WritePayload(const char *p_data)
    {
      memcpy(&m_storage[sizeof(Header)], p_data, m_header.payloadSize);
    }

    /**
     * @brief Replace packet's opcode and save it in buffer
     */
    void    ByteBuffer::ReplaceOpcode(uint16_t p_opcode)
    {
      m_header.opcode = p_opcode;
      WriteHeader();
    }

    /**
     * @brief Assign packet's special id and save it in buffer
     *
     * @param p_id special id to assign
     */
    void    ByteBuffer::AssignSpecialId(uint32_t p_id)
    {
      m_header.specialId = p_id;
      WriteHeader();
    }

    /**
     * @brief Finalize packet construction
     * @details Compress() packet and write his header
     */
    void		ByteBuffer::Release()
    {
      m_header.realPayloadSize = m_header.payloadSize;
      Compress();
      WriteHeader();
    }

    /**
     * @brief Compress packet's payload
     */
    void ByteBuffer::Compress()
    {
      uLongf destSize;
      const char *buffer;

      buffer = Utils::Zip::Compress((char *)(&m_storage[sizeof(Header)]), m_header.payloadSize, destSize);
      m_header.payloadSize = static_cast<int32_t>(destSize);
      m_storage.resize(GetSize());
      WritePayload(buffer);
      m_header.isCompressed = true;
      delete[] buffer;
    }

    /**
     * @brief Decompress packet's payload
     */
    void ByteBuffer::Decompress()
    {
      if (!m_header.isCompressed)
        return;
      uLongf destSize = static_cast<uLongf>(m_header.realPayloadSize);
      const char *buffer;

      buffer = Utils::Zip::Uncompress((char *)(&m_storage[sizeof(Header)]),
                                      static_cast<uLongf>(m_header.payloadSize), destSize);
      m_header.payloadSize = static_cast<int32_t>(destSize);
      m_storage.resize(GetSize());
      WritePayload(buffer);
      m_header.isCompressed = false;
      m_header.realPayloadSize = m_header.payloadSize;
      delete[] buffer;
    }

    std::string ByteBuffer::ToString()
    {
      std::string value;

      for (int i = 0; i < m_header.payloadSize; ++i)
      {
        value += "[";
        value += std::to_string(m_storage[i]);
        value += "]\n";
      }
      return (value);
    }

    ByteBuffer::Exception::Exception(int32_t p_size, int32_t p_pos, const std::string &p_message) throw()
    {
      m_message = "{size: " + std::to_string(p_size);
      m_message += ", position: " + std::to_string(p_pos) + "} ";
      m_message += p_message.c_str();
    }
  }
}
