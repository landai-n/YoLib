/*!
* \file Hash.h
* \author Nathanael Landais
*/

#ifndef		YO_HASH_H
# define	YO_HASH_H

# include	<cstdint>
# include	<string>

namespace	Yo
{
  namespace	Utils
  {
    class	Hash
    {
    public:
      enum Format
      {
        Hexadecimal,
        Base64
      };
      static std::string 	GenerateRandomString(uint16_t);
      static char 		    GenerateRandomChar();
      static std::string	GenerateSha1(const std::string &, const std::string &, Format = Hexadecimal);
    };
  }
}

#endif
