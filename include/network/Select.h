/*!
* \file Select.h
* \author Nathanael Landais
*/

#ifndef		YO_SELECT_H
# define	YO_SELECT_H

# include	<set>
# include <condition_variable>
# include <list>
# include	"ASocket.h"

namespace	Yo
{
  namespace	Network
  {
    /**
     * @brief C++ implementation of select()
     *
     */
    class	Select
    {
    public:
      enum	Mode
      {
        READ,
        WRITE
      };
      Select(Select::Mode);
      void				AddSocketASync(ASocket&);
      void        PopSocketFromASync(ASocket&);
      void				AddSocket(ASocket&);
      void				PopSocket(ASocket&);
      bool				IsSet(ASocket&);
      bool 				Wait();
      void				SetTimeout(int, int);
    private:
      void				PopSocket_intern(ASocket&);
      void				AddSocket_intern(ASocket&);
      void				Reset();
      /**
       * @brief Conditional class used to check if there is socket to monitor
       */
      class SocketChecker
      {
        std::set<SOCKET> &m_set;
      public:
        SocketChecker(std::set<SOCKET> &set) : m_set(set) {}
        bool operator()() { return (m_set.empty() == false); }
      };
      int                       GetLastSocket();
      std::mutex                m_lock;
      std::mutex                m_lockASync;
      fd_set				            m_fdList;
      struct timeval	          m_timeout;
      struct timeval	          m_timeoutSave;
      Select::Mode		          m_mode;
#if !defined(WIN32) || !defined(_WIN32)
      SOCKET				            last_fd;
#endif
      SocketChecker             m_checkSocket;
      std::condition_variable   m_socketWaiter;
      std::set<SOCKET>	        m_fd;
      std::set<ASocket*>        m_tmpSocketList;
    };
  }
}
#endif
