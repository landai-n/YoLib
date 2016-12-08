/*!
* \file Dev.cpp
* \author Nathanael Landais
*/

#if defined(linux) || defined(__unix) || defined(__linux)
#include 	<execinfo.h>
#endif
#include	<string.h>
#include  	<assert.h>
#ifdef NDEBUG
#undef NDEBUG
#include  	<assert.h>
#define NDEBUG
#endif
#include	"Dev.h"
#include <cxxabi.h>

namespace Yo
{
  namespace Utils
  {
    std::string Dev::GetStackTrace(std::string p_eol)
    {
      std::string trace;
#if defined(linux) || defined(__unix) || defined(__linux)
      void *buffer[100];
      int  nbSymbols;
      char **strings;

      nbSymbols    = backtrace(buffer, 100);
      if ((strings = backtrace_symbols(buffer, nbSymbols)) == nullptr)
        return (strerror(errno));

      for (int i = 0; i < nbSymbols; ++i)
      {
        char *mangled = 0, *offsetBegin = 0, *offsetEnd = 0;

        for (char *p = strings[i]; *p; ++p)
        {
          if (*p == '(')
            mangled = p;
          else if (*p == '+')
            offsetBegin = p;
          else if (*p == ')')
          {
            offsetEnd = p;
            break;
          }
        }
        if (mangled && offsetBegin && offsetEnd &&
            mangled < offsetBegin)
        {
          *mangled++     = '\0';
          *offsetBegin++ = '\0';
          *offsetEnd++   = '\0';

          int status;
          char * real_name = abi::__cxa_demangle(mangled, 0, 0, &status);

          if (status == 0)
            trace += std::string(strings[i]) + ": " +
                     std::string(real_name) + "+" + std::string(offsetBegin) + std::string(offsetEnd);
          else
            trace += std::string(strings[i]) + " : "
                     + std::string(mangled) + "+" + std::string(offsetBegin) + std::string(offsetEnd);
          free(real_name);
        }
        else
          trace += std::string(strings[i]);
        if (i + 1 < nbSymbols)
          trace += p_eol;
      }
      free(strings);
#endif
      return (trace);
    }
  }
}
