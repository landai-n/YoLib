/*!
* \file Thread.h
* \author Nathanael Landais
*/

#ifndef		YO_THREAD_H
# define	YO_THREAD_H

# include	<iostream>
# include	<mutex>
# include	<functional>
# include	<memory>
# include	<condition_variable>
# include <signal.h>
# include	"../utils/Dev.h"
# include	"../utils/Log.h"

namespace Yo
{
  template <class Obj, typename Func, typename ArgType>
  class ThreadPool;

  /**
   * @brief Class used to thread ThreadPool's functions
   */
  template <class Obj, typename Func, typename ArgType>
  class Thread
  {
    /**
     * @brief Conditional class check if Thread is waiting a function
     */
    class RunLocker
    {
    public:
      volatile bool &m_wait;
      RunLocker(volatile bool &wait) :m_wait(wait) {}
      bool operator()() { return (!m_wait); }
    };
     /**
     * @brief Conditional class check if Thread is not waiting a function
     */
    class InvRunLocker
    {
    public:
      volatile bool &m_wait;
      InvRunLocker(volatile bool &wait) :m_wait(wait) {}
      bool operator()() { return (m_wait); }
    };
  protected:
    volatile bool                                   waitFunction;
    std::thread                                     *m_thread;
    volatile bool                                   m_isFree;
    std::shared_ptr<ThreadPool<Obj, Func, ArgType>> &m_pool;
    std::function<void()>                           m_lambdaFunc;
    RunLocker                                       m_runLocker;
    InvRunLocker                                    m_invRunLocker;
    std::condition_variable                         m_runWaiter;
    std::condition_variable                         m_invRunWaiter;
    std::mutex                                      m_lock;

    /**
     * @brief Get associated pool
     * @return Associated pull
     */
    ThreadPool< Obj, Func, ArgType > *GetPool() const
    {
      return (m_pool);
    }

    static Thread<Obj, Func, ArgType> *GetThis(Thread<Obj, Func, ArgType>  *assign = nullptr)
    {
      static thread_local Thread<Obj, Func, ArgType>  *me = nullptr;

      if (assign != nullptr)
        me = assign;
      return (me);
    }

    /**
     * @brief Lock thread
     */
    void	Lock(std::unique_lock<std::mutex> &l)
    {
      m_pool->IndicateFree(this);
      m_runWaiter.wait(l, m_runLocker); // wait while waitFunction == true (unlock l while waiting, then relock it)
      waitFunction = true;
      m_invRunWaiter.notify_all();
    }

    /**
     * @brief Unlock thread
     */
    void	Unlock(std::unique_lock<std::mutex> &l)
    {
      m_invRunWaiter.wait(l, m_invRunLocker); // wait while waitFunction == false (unlock l while waiting, then relock it)
      waitFunction = false;
      m_runWaiter.notify_all();
    }

    /**
    * @brief Catch Thread's crash and addempt recovery
    */
    static void SignalHandler(int)
    {
      static std::thread::id prevThread;
      static const int maxLuck = 0;
      static auto start = std::chrono::high_resolution_clock::now();
      static int luck = maxLuck;
      auto now = std::chrono::high_resolution_clock::now();

      if (std::this_thread::get_id() == prevThread)
        return;
      prevThread = std::this_thread::get_id();
      if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() < 1000)
        luck--;
      else
      {
        luck = maxLuck;
        start = std::chrono::high_resolution_clock::now();
      }
#ifdef SEGREPORT
      Utils::Log::writeError("SIGSEGV::Thread", "SEGMENTATION FAULT:\n" + Utils::Dev::GetStackTrace());
#endif
      if (luck >= 0)
      {
        Utils::Log::writeInfo("Restart crashed thread");
        GetThis()->AskForRestart();
      }
      else
      {
        Utils::Log::writeError("SIGSEGV::Thread", "All recovery chance exhausted. Terminate.");
        std::terminate();
      }
    }

    /**
     * @brief Thread's loop, receive and execute attached functions
     */
    void	Run()
    {
      GetThis(this);
      signal(SIGSEGV, SignalHandler);
      std::unique_lock<std::mutex> l(m_lock);

      while (true)
      {
        this->Lock(l);
        if (m_lambdaFunc == nullptr)
          break;

        m_lambdaFunc();
        this->m_lambdaFunc = nullptr;
      }
    }

    /**
     * @brief Try to restart this Thread
     */
    void AskForRestart()
    {
      try
      {
        if (this == nullptr)
        {
          Utils::Log::writeError("Thread::AskForRestart", "Unable to restart this thread. Terminate.");
          std::terminate();
        }
        auto me = m_thread;
        Restart();
        me->detach();
        delete (me);
      }
      catch (...)
      {

      }
    }

  public:
    /**
     * @brief Restart this Thread
     */
    void Restart()
    {
      waitFunction = true;
      m_isFree = true;
      m_lambdaFunc = nullptr;
      Start();
    }

    Thread(std::shared_ptr<ThreadPool< Obj, Func, ArgType >> &p_pool) :
      waitFunction(true), m_runLocker(waitFunction), m_invRunLocker(waitFunction), m_isFree(true), m_pool(p_pool)
    {
      m_lambdaFunc = nullptr;
      m_thread = nullptr;
    }

    virtual ~Thread()
    {
      std::unique_lock<std::mutex> l(m_lock);

      Unlock(l);
      Unlock(l);
      if (m_thread)
        m_thread->join();
      delete (m_thread);
    }

    /**
     * @brief Start thread
     */
    void	Start()
    {
      m_thread = new std::thread(&Thread::Run, this);
    }

    /**
     * @brief Execute parameter p_lambdaFunc in this Thread
     *
     */
    void	Execute(std::function<void()> &p_lambdaFunc)
    {
      std::unique_lock<std::mutex> l(m_lock);

      m_lambdaFunc = p_lambdaFunc;
      Unlock(l);
    }
  };
}
#endif
