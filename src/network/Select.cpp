/*!
* \file Select.cpp
* \author Nathanael Landais
*/

#include	"Log.h"
#include	"Select.h"
#include	"Dev.h"

namespace	Yo
{
  namespace	Network
  {
    /**
     * @brief Initialize Select
     *
     * @param p_mode Select's mode
     */
    Select::Select(Select::Mode p_mode) : m_checkSocket(m_fd)
    {
      std::unique_lock<std::mutex> l(m_lock);
      m_timeout.tv_sec = 0;
      m_timeout.tv_usec = 0;
      m_timeoutSave = m_timeout;
      m_mode = p_mode;
#if !defined(WIN32) || !defined(_WIN32)
      last_fd = INVALID_SOCKET;
#endif
      FD_ZERO(&m_fdList);
    }

    int      Select::GetLastSocket()
    {
#if defined(WIN32) || defined(_WIN32)
      return (static_cast<int>(m_fd.size()));
#elif !defined(WIN32) || !defined(_WIN32)
      return (int(last_fd) + 1);
#endif
    }

    /**
     * @brief Add a socket to monitor
     * @details p_socket will be added on next Wait() call
     *
     * @param p_socket Socket to add
     */
    void      Select::AddSocketASync(ASocket &p_socket)
    {
      std::unique_lock<std::mutex> l(m_lockASync);

      m_tmpSocketList.insert(&p_socket);
    }

    /**
     * @brief Add a socket to monitor
     * @details p_socket will be removed on next Wait() call
     *
     * @param p_socket Socket to add
     */
    void      Select::PopSocketFromASync(ASocket &p_socket)
    {
      std::unique_lock<std::mutex> l(m_lockASync);

      m_tmpSocketList.erase(&p_socket);
    }

    /**
     * @brief Add a socket to monitor
     * @details If another thread is currently in a Wait(),
     *  will block until end of Wait()
     * @param p_socket Socket to add
     */
    void      Select::AddSocket(ASocket &p_socket)
    {
      std::unique_lock<std::mutex> l(m_lock);

      AddSocket_intern(p_socket);
    }

    void      Select::AddSocket_intern(ASocket &p_socket)
    {
      SOCKET  sock = p_socket.GetSock();
      FD_SET(sock, &m_fdList);
#if !defined(WIN32) || !defined(_WIN32)
      if (last_fd == INVALID_SOCKET || sock > last_fd)
        last_fd = sock;
#endif
      m_fd.insert(sock);
      m_socketWaiter.notify_one();
    }

    /**
     * @brief Check if a socket has been set in previous select
     *
     * @param p_socket Socket to check
     * @return true if socket has been set, false otherwise
     */
    bool    Select::IsSet(ASocket &p_socket)
    {
      std::unique_lock<std::mutex> l(m_lock);

      if (p_socket.IsOpen() == false)
        return (false);
      if (FD_ISSET(p_socket.GetSock(), &m_fdList))
      {
        FD_CLR(p_socket.GetSock(), &m_fdList);
        PopSocket_intern(p_socket);
        return (true);
      }
      return (false);
    }

    /**
     * @brief Reset Select attributes to their post Wait() state
     *
     */
    void    Select::Reset()
    {
      FD_ZERO(&m_fdList);
#if !defined(WIN32) || !defined(_WIN32)
      last_fd = 0;
#endif
      if (m_fd.empty())
        return;
      for (auto elem = m_fd.begin(); elem != m_fd.end(); ++elem)
      {
        FD_SET(*elem, &m_fdList);
#if !defined(WIN32) || !defined(_WIN32)
        if (*elem > last_fd)
          last_fd = *elem;
#endif
      }
      m_timeout = m_timeoutSave;
    }

    /**
     * @brief Remove a socket from monitoring
     * @details If another thread is currently in a Wait(),
     *  will block until end of Wait()
     *
     * @param p_socket Socket to remove
     */
    void    Select::PopSocket(ASocket &p_socket)
    {
      std::unique_lock<std::mutex> l(m_lock);

      PopSocket_intern(p_socket);
      PopSocketFromASync(p_socket);
    }

    void    Select::PopSocket_intern(ASocket &p_socket)
    {
      SOCKET  sock = p_socket.GetSock();

      auto targetfd = m_fd.find(sock);

      if (targetfd == m_fd.end())
        return;

      if (m_fd.empty())
        last_fd = INVALID_SOCKET;
      else if (*targetfd == last_fd)
      {
        targetfd--;
        last_fd = *targetfd;
      }
      m_fd.erase(sock);
    }

    /**
     * @brief Set Wait() timeout
     *
     * @param sec Number of seconds
     * @param usec Number of micro seconds
     */
    void    Select::SetTimeout(int sec, int usec)
    {
      std::unique_lock<std::mutex> l(m_lock);
      m_timeout.tv_sec = sec;
      m_timeout.tv_usec = usec;
      m_timeoutSave = m_timeout;
    }

    /**
     * @brief Wait for an event on a monitored socket
     *
     * @return true if an event has been detected, false otherwise
     */
    bool    Select::Wait()
    {
      std::unique_lock<std::mutex> l(m_lock);

      m_lockASync.lock();
      for (auto socket = m_tmpSocketList.begin(); socket != m_tmpSocketList.end(); ++socket)
        AddSocket_intern(**socket);
      m_tmpSocketList.clear();
      m_lockASync.unlock();

      int   ret = -2;

      m_socketWaiter.wait_for(l, std::chrono::seconds(1), m_checkSocket);
#if defined(WIN32) || defined(_WIN32)
      if (GetLastSocket() == 0)
        return (false);
#elif !defined(WIN32) || !defined(_WIN32)
      if (last_fd == INVALID_SOCKET)
        return (false);
#endif
      this->Reset();
      if (m_mode == Mode::READ)
        ret = select(GetLastSocket(), &m_fdList, NULL, NULL, &m_timeout);
      else if (m_mode == Mode::WRITE)
        ret = select(GetLastSocket(), NULL, &m_fdList, NULL, &m_timeout);

      switch (ret)
      {
        case 0:
          return (false);
        case -1:
          Utils::Log::writeError("Select::Wait()");
          return (false);
        case -2:
          throw std::runtime_error("Select::Wait(): Invalid select mode");
        default:
          return true;
      }
    }
  }
}
