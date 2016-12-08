/*!
* \file Dev.h
* \author Nathanael Landais
*/

#ifndef		YO_DEV_H
# define	YO_DEV_H

# include	<thread>
# include	<chrono>
# include	<cstdint>
# include <iostream>
# include <mutex>
# include "Log.h"

#define ASSERT(x) { if (!x) { std::cerr << "Assertion " << __STRING(x) << " failed in " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__ << std::endl; abort(); } }
#define EXPECT(x) { if (!x) { std::cerr << "Expectation " << __STRING(x) << " failed in " << __PRETTY_FUNCTION__ << " at " << __FILE__ << ":" << __LINE__ << std::endl; } }

namespace Yo
{
  namespace Utils
  {
    class Dev
    {
    public:
      static inline void Sleep(uint32_t p_ms) { std::this_thread::sleep_for(std::chrono::milliseconds(p_ms)); }
      static std::string GetStackTrace(std::string = "\r\n");
    private:

    };
  }
}

#endif
