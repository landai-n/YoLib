/*!
* \file ByteBuffer.h
* \author Nathanael Landais
*/

#ifndef		YO_BYTEBUFFER_H
# define	YO_BYTEBUFFER_H

# include	<vector>
# include	<cstdint>
# include	<string>
# include	<cstring>
# include	<stdexcept>
# include <zconf.h>
# include <list>
# include <memory>
# include "../utils/Log.h"

namespace Yo
{
  namespace Network
  {
    /**
     * @brief Standard network packet storage class
     */
    class ByteBuffer
    {
    public:
      class Stack
      {
      public:
        Stack() { m_size = 0; }
        bool Empty() const { return m_buffers.empty(); }
        void Push(ByteBuffer *p_buffer)
        {
          m_buffers.push_back(std::shared_ptr<ByteBuffer>(p_buffer));
          m_size += p_buffer->GetSize();
        }
        void PushReverse(ByteBuffer *p_buffer)
        {
          m_buffers.push_front(std::shared_ptr<ByteBuffer>(p_buffer));
          m_size += p_buffer->GetSize();
        }
        const std::list<std::shared_ptr<ByteBuffer>> GetList()
        {
          return (m_buffers);
        }
        ssize_t GetSize() const { return (m_size); }
        void Clear()
        {
          m_size = 0;
          m_buffers.clear();
        }
      private:
        std::list<std::shared_ptr<ByteBuffer>> m_buffers;
        ssize_t m_size;
      };

      /**
       * @brief Packet header
       */
      typedef struct __attribute__((__packed__))
      {

        int32_t  payloadSize;
        int32_t  realPayloadSize;
        /**
         * @brief id used between server and minion to trace associated client
         */
        uint32_t specialId;
        bool     isCompressed;
        uint16_t error;
        uint16_t opcode;
      }       Header;

      class Exception : public std::exception
      {
      public:
        Exception(int32_t, int32_t, const std::string &) throw();

        virtual const char *what() const throw()
        { return (m_message.c_str()); };

        virtual ~Exception() throw()
        { };
      private:
        std::string m_message;
      };

      ByteBuffer(uint16_t, uint16_t = 0);

      ByteBuffer(char *, Header);

      ~ByteBuffer();

      const char *GetBuffer() const;

      int32_t   GetSize() const;

      ByteBuffer &operator<<(uint8_t);

      ByteBuffer &operator<<(uint16_t);

      ByteBuffer &operator<<(uint32_t);

      ByteBuffer &operator<<(uint64_t);

      ByteBuffer &operator<<(int8_t);

      ByteBuffer &operator<<(bool);

      ByteBuffer &operator<<(int16_t);

      ByteBuffer &operator<<(int32_t);

      ByteBuffer &operator<<(int64_t);

      ByteBuffer &operator<<(float);

      ByteBuffer &operator<<(double);

      ByteBuffer &operator<<(const std::string &);

      ByteBuffer &operator>>(uint8_t &);

      ByteBuffer &operator>>(bool &);

      ByteBuffer &operator>>(uint16_t &);

      ByteBuffer &operator>>(uint32_t &);

      ByteBuffer &operator>>(uint64_t &);

      ByteBuffer &operator>>(int8_t &);

      ByteBuffer &operator>>(int16_t &);

      ByteBuffer &operator>>(int32_t &);

      ByteBuffer &operator>>(int64_t &);

      ByteBuffer &operator>>(float &);

      ByteBuffer &operator>>(double &);

      ByteBuffer &operator>>(std::string &);

      template<typename T>
      void       WriteAtPosition(T value, int32_t position)
      {
        memcpy(&m_storage[position], &value, sizeof(value));
      }

      void        Release();

      void        ReplaceOpcode(uint16_t);
      void        AssignSpecialId(uint32_t);

      uint16_t    GetOpcode() const { return (m_header.opcode); }
      uint32_t    GetSpecialId() const { return (m_header.specialId); }
      uint16_t    GetError() const { return (m_header.error); }

      std::string ToString();

    private:
      std::vector<uint8_t> m_storage;
      int32_t              m_pos;
      Header               m_header;

      void                 Compress();
      void                 Decompress();
      void                 WriteHeader();
      void                 WritePayload(const char *);

    public:
      /**
       * @brief Append p_var at end of packet
       *
       * @param p_var Variable to append
       */
      template<typename T>
      inline void Append(T p_var)
      {
        m_header.payloadSize += sizeof(p_var);
        m_storage.resize(GetSize());
        memcpy(&m_storage[GetSize() - sizeof(p_var)], &p_var, sizeof(p_var));
      }

      /**
       * @brief Read value T from packet
       *
       * @tparam T value type
       * @return Value
       */
      template<typename T>
      T Read()
      {
        T value = 0;

        Read((void *)&value, sizeof(value));
        return (value);
      }

      void Read(void *p_value, size_t p_size)
      {
        if (static_cast<int32_t>(m_pos + p_size) > GetSize())
          return;
        memcpy(p_value, &m_storage[m_pos], p_size);
        m_pos += p_size;
      }

      /**
       * @brief Replace cursor position at begining of the payload
       */
      void ResetPos()
      {
        m_pos  = sizeof(Header);
      }
    };
  }
}
#endif
