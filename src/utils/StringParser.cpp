/*!
* \file StringParser.cpp
* \author Nathanael Landais
*/

#include "StringParser.h"

namespace Yo
{
  namespace Utils
  {
    std::string StringParser::GetFirstLine(const std::string &p_text, const std::string &p_match)
    {
      ulong beginPos = p_text.find(p_match);
      ulong length   = 0;

      if (beginPos == p_text.npos)
        return ("");
      for (; beginPos + length < p_text.size() && p_text[beginPos + length] != '\n'; ++length);
      if (p_text[beginPos + length - 1] == '\r')
        length--;
      return (p_text.substr(beginPos, length));
    }

    std::string StringParser::GetWord(const std::string &p_text, uint32_t p_index)
    {
      std::istringstream stream(p_text);
      std::string word;

      for (uint32_t i = 0; i < p_index; ++i)
      {
        word.erase();
        if (!(stream >> word))
          break;
      }

      return (word);
    }
  }
}
