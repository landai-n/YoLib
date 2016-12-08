/*!
* \file ThreadPool.h
* \author Nathanael Landais
*/

#ifndef		YO_THREADPOOL_H
# define	YO_THREADPOOL_H

# include	<thread>
# include	<future>
# include	<list>
# include	<functional>
# include	<mutex>
# include	"../utils/Log.h"
# include	"Thread.h"

namespace Yo
{
  /**
   * @brief Thread pool implementation
   *
   * @tparam Obj Object Type used to execute functions in threads
   * @tparam Func Method Type to execute
   * @tparam ArgType Method arguments
   */
  template<class Obj, typename Func, typename ArgType>
  class ThreadPool
  {
    class RunLocker
    {
    public:
      const std::list<Thread<Obj, Func, ArgType> *> &m_available;
      const uint32_t                                &m_nbMaxThead;

      RunLocker(const std::list<Thread<Obj, Func, ArgType> *> &p_available, const uint32_t &nbMax)
          : m_available(p_available), m_nbMaxThead(nbMax)
      { }

      bool operator()()
      { return (m_available.size() == m_nbMaxThead); }
    };

    class InvRunLocker
    {
    public:
      const std::list<Thread<Obj, Func, ArgType> *> &m_available;
      const uint32_t                                &m_nbMaxThead;

      InvRunLocker(const std::list<Thread<Obj, Func, ArgType> *> &p_available, const uint32_t &nbMax)
          : m_available(p_available), m_nbMaxThead(nbMax)
      { }

      bool operator()()
      { return (!m_available.empty()); }
    };

  protected:
    uint32_t                                        m_nbMaxThreads;
    Thread<Obj, Func, ArgType>                      **m_threadPool;
    std::shared_ptr<ThreadPool<Obj, Func, ArgType>> m_this;
    std::mutex                                      m_lock;
    std::condition_variable                         m_runningWait;
    RunLocker                                       m_runningLock;
    std::condition_variable                         m_invRunningWait;
    InvRunLocker                                    m_invRunningLock;
    std::list<Thread<Obj, Func, ArgType> *>         m_availableThread;

  public:
                 ThreadPool(uint32_t, const std::string &);

    virtual ~ThreadPool()
    { }

    static void null_deleter(ThreadPool *)
    { }

    virtual void Start();

    void         ExecuteAll(Obj, Func, ArgType);

    bool Execute(Obj, Func, ArgType);

    bool TryExecute(std::function<void()> &);

    void         Stop();

    inline uint32_t GetNbThreadAvailable()
    {
      std::unique_lock<std::mutex> l(m_lock);
      return (m_availableThread.size());
    }

    uint32_t GetCapacity() const
    { return (m_nbMaxThreads); }

    void         IndicateFree(Thread<Obj, Func, ArgType> *);
  };

  /**
   * @brief Constructor
   *
   * @param p_nbMaxThreads Pool size
   * @param p_reason Pool name
   */
  template<class Obj, typename Func, typename ArgType>
  ThreadPool<Obj, Func, ArgType>::ThreadPool(uint32_t p_nbMaxThreads, const std::string &p_reason)
      : m_runningLock(m_availableThread, m_nbMaxThreads), m_invRunningLock(m_availableThread, m_nbMaxThreads)
  {
    Utils::Log::writeInfo("Create " + std::to_string(p_nbMaxThreads) + " threads (" + p_reason + ")", true);
    m_nbMaxThreads = p_nbMaxThreads;
    m_this         = std::shared_ptr<ThreadPool<Obj, Func, ArgType>>(this, null_deleter);
    m_threadPool   = nullptr;
  }

  /**
   * @brief Execute given function on every available thread
   * @details [long description]
   *
   * @param p_obj Object to use
   * @param p_func Method of p_obj to use
   * @param p_arg arguments of p_func to use
   */
  template<class Obj, typename Func, typename ArgType>
  void  ThreadPool<Obj, Func, ArgType>::ExecuteAll(Obj p_obj, Func p_func, ArgType p_arg)
  {
    for (uint32_t i = 0; i < m_nbMaxThreads; ++i)
      Execute(p_obj, p_func, p_arg);
  }

  /**
   * @brief Try to execute given function
   * @details Return if no thread are available
   *
   * @param p_lambdaFunc Function to execute
   * @return true if p_lambdaFunc has been executed, false otherwise
   */
  template<class Obj, typename Func, typename ArgType>
  bool  ThreadPool<Obj, Func, ArgType>::TryExecute(std::function<void()> &p_lambdaFunc)
  {
    std::unique_lock<std::mutex> l(m_lock);

    if (m_availableThread.empty())
      return (false);
    auto thread = m_availableThread.front();

    m_availableThread.pop_front();
    thread->Execute(p_lambdaFunc);
    return (true);
  }

  /**
   * @brief Wait for an available thread and execute p_lambdaFunc on it
   *
   * @param p_lambdaFunc Function to execute
   * @return Success
   */
  template<class Obj, typename Func, typename ArgType>
  bool  ThreadPool<Obj, Func, ArgType>::Execute(Obj p_obj, Func p_func, ArgType p_arg)
  {
    std::unique_lock<std::mutex> l(m_lock);

    m_invRunningWait.wait(l, m_invRunningLock); //wait while no thread available
    auto thread = m_availableThread.front();

    m_availableThread.pop_front();
    std::function<void()> ptr = [thread, p_obj, p_func, p_arg]()
    {
      ((p_obj)->*(p_func))(p_arg, thread);
    };

    l.unlock();
    thread->Execute(ptr);
    return (true);
  }

  /**
   * @brief Specify that a thread is available
   *
   * @param p_thread Thread to signal
   */
  template<class Obj, typename Func, typename ArgType>
  void        ThreadPool<Obj, Func, ArgType>::IndicateFree(Thread<Obj, Func, ArgType> *p_thread)
  {
    std::unique_lock<std::mutex> l(m_lock);

    m_availableThread.push_back(p_thread);
    m_invRunningWait.notify_all();
    m_runningWait.notify_all();
  }

  /**
   * @brief Start threads
   */
  template<class Obj, typename Func, typename ArgType>
  void  ThreadPool<Obj, Func, ArgType>::Start()
  {
    m_threadPool = new Thread<Obj, Func, ArgType> *[m_nbMaxThreads];
    for (uint32_t i = 0; i < m_nbMaxThreads; ++i)
    {
      Thread<Obj, Func, ArgType> *l_thread = new Thread<Obj, Func, ArgType>(m_this);

      l_thread->Start();
      m_threadPool[i] = l_thread;
    }
  }

  /**
   * @brief Stop all threads
   */
  template<class Obj, typename Func, typename ArgType>
  void  ThreadPool<Obj, Func, ArgType>::Stop()
  {
    std::unique_lock<std::mutex> l(m_lock);

    m_runningWait.wait(l, m_runningLock);   // wait while at least one thread is used
    if (m_threadPool != nullptr)
    {
      for (uint32_t i = 0; i < m_nbMaxThreads; ++i)
        delete m_threadPool[i];
      delete[] m_threadPool;
    }
  }
}

#endif
