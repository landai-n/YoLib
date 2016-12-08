/*!
* \file StringParser.h
* \author Nathanael Landais
*/

#ifndef YO_STRINGPARSER_H
#define YO_STRINGPARSER_H

#include <sstream>

namespace Yo
{
  namespace Utils
  {
    class StringParser
    {
    public:
      static std::string GetFirstLine(const std::string &, const std::string &);
      static std::string GetWord(const std::string &, uint32_t p_index);
    };
  }
}

#endif
