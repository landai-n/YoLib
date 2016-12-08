/*!
* \file SharedThreadPool.h
* \author Nathanael Landais
*/

#ifndef		YO_SHAREDTHREADPOOL_H
# define	YO_SHAREDTHREADPOOL_H

# include	<memory>
# include	"ThreadPool.h"
# include	"../utils/Dev.h"

namespace Yo
{
  /**
   * @brief ThreadPool manager allowing to set a pool usage limit
   */
  template <typename Obj, typename Func, typename ArgType>
  class SharedThreadPool
  {
  public:
    /**
     * @brief Initialization
     *
     * @param p_pool Pool to manage
     * @param p_percent Maximal use percentage
     */
    SharedThreadPool(std::shared_ptr<ThreadPool<Obj, Func, ArgType>> p_pool, double p_percent) : m_pool(p_pool)
    {
      m_allowedPercent = p_percent;
    }

    /**
     * @brief Execute a parameter function
     * @details Function is executed on an available thread of the thread pool
     *  if there is one available and maximal use percentage has not been reached,
     *   will be executed on current thread otherwise
     * @param p_lambdaFunc function to execute
     */
    void Execute(std::function<void()> &p_lambdaFunc)
    {
      if (1 - (double(m_pool->GetNbThreadAvailable()) / double(m_pool->GetCapacity())) < m_allowedPercent)
        if (m_pool->TryExecute(p_lambdaFunc) == true)
          return;
      p_lambdaFunc();
    }

  private:
    std::shared_ptr<ThreadPool<Obj, Func, ArgType>> 	m_pool;
    double						m_allowedPercent;
    std::mutex						m_lock;
  };
}

#endif
