#include	<iostream>
#include	<cstring>
#if defined(__WIN32) || defined (WIN32)
#include	<Windows.h>
#endif
#include	"Log.h"
#include  "Dev.h"

namespace Yo
{
  namespace Utils
  {
    std::mutex Log::lock;
    std::string Log::cache = "";
    bool Log::Verbose = true;

    bool Log::writeError(const std::string &p_func, const std::string &p_error)
    {
      lock.lock();
      std::cerr << "[ERROR] " << p_func << ": " << p_error << std::endl;
      lock.unlock();

      return (false);
    }

    bool Log::writeError(const std::string &p_func)
    {
      #if defined(__WIN32) || defined (WIN32)
      Log::writeError(p_func, std::to_string(WSAGetLastError()));
      #else
      Log::writeError(p_func, strerror(errno));
      #endif
      return (false);
    }

    void Log::writeInfo(const std::string &p_message, bool p_verbose)
    {
      if (p_verbose && !Verbose)
        return;
      lock.lock();
      std::cout << "[INFO] " << p_message << std::endl;
      lock.unlock();
    }

    void Log::writeWarning(const std::string &p_message)
    {
      lock.lock();
      std::cout << "[WARN] " << p_message << std::endl;
      lock.unlock();
    }

    void Log::writeDebug(const std::string &p_message, bool p_cache)
    {
      lock.lock();
      if (p_cache)
        cache += "[DEBUG] " + p_message + "\n";
      else
        std::cout << "[DEBUG] " << p_message << std::endl;
      lock.unlock();
    }

    void Log::Flush()
    {
      lock.lock();
      std::cout << cache << std::flush;
      cache.clear();
      lock.unlock();
    }
  }
}
