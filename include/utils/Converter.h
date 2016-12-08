/*!
* \file Converter.h
* \author Nathanael Landais
*/

#ifndef		YO_CONVERTER_H
# define	YO_CONVERTER_H

# include <string>
# include <vector>
# include	<sstream>

namespace	Yo
{
  namespace	Utils
  {
    class Converter
    {
    public:
      template<typename T>
      static T ParseString(const std::string &p_str)
      {
        T value;

        std::istringstream(p_str) >> value;
        return (value);
      }

      static std::basic_string<char> Base64Encode(std::vector<unsigned char> inputBuffer)
      {
        const static char       encodeLookup[]      = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        const static char       padCharacter        = '=';
        std::basic_string<char> encodedString;
        encodedString.reserve(((inputBuffer.size() / 3) + (inputBuffer.size() % 3 > 0)) * 4);
        long                    temp;
        std::vector<unsigned char>::iterator cursor = inputBuffer.begin();
        for (size_t idx = 0; idx < inputBuffer.size() / 3; idx++)
        {
          temp = (*cursor++) << 16; //Convert to big endian
          temp += (*cursor++) << 8;
          temp += (*cursor++);
          encodedString.append(1, encodeLookup[(temp & 0x00FC0000) >> 18]);
          encodedString.append(1, encodeLookup[(temp & 0x0003F000) >> 12]);
          encodedString.append(1, encodeLookup[(temp & 0x00000FC0) >> 6]);
          encodedString.append(1, encodeLookup[(temp & 0x0000003F)]);
        }
        switch (inputBuffer.size() % 3)
        {
          case 1:
            temp = (*cursor++) << 16; //Convert to big endian
            encodedString.append(1, encodeLookup[(temp & 0x00FC0000) >> 18]);
            encodedString.append(1, encodeLookup[(temp & 0x0003F000) >> 12]);
            encodedString.append(2, padCharacter);
            break;
          case 2:
            temp = (*cursor++) << 16; //Convert to big endian
            temp += (*cursor++) << 8;
            encodedString.append(1, encodeLookup[(temp & 0x00FC0000) >> 18]);
            encodedString.append(1, encodeLookup[(temp & 0x0003F000) >> 12]);
            encodedString.append(1, encodeLookup[(temp & 0x00000FC0) >> 6]);
            encodedString.append(1, padCharacter);
            break;
        }
        return (encodedString);
      }
    };
  }
}

#endif
