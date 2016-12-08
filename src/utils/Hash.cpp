/*!
* \file Hash.cpp
* \author Nathanael Landais
*/

#include	"Hash.h"
#include	"Log.h"
#include	<random>
#include	<algorithm>
#include	<openssl/sha.h>
#include	<iostream>
#include	<sstream>
#include	<iomanip>
#include        <Converter.h>

namespace	Yo
{
  namespace	Utils
  {
    char Hash::GenerateRandomChar()
    {

      static const char charSet[] =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
      static const size_t max = (sizeof(charSet) - 2);
      static std::uniform_int_distribution<int> distrib(0, max);
      static std::default_random_engine generator;
      return (charSet [ distrib(generator) ]);
    }

    std::string Hash::GenerateRandomString(uint16_t p_size)
    {
      std::default_random_engine generator;
      std::uniform_int_distribution<int> distribution(1,6);

      std::string str(p_size, 0);

      std::generate_n(str.begin(), p_size, &Hash::GenerateRandomChar);

      return(str);
    }

    static std::string GetHexRepresentation(const unsigned char * Bytes, size_t Length)
    {
      std::ostringstream os;
      os.fill('0');
      os << std::hex;
      for (const unsigned char *ptr = Bytes; ptr < Bytes + Length; ptr++)
        os << std::setw(2) << (unsigned int) *ptr;
      return (os.str());
    }

    std::string Hash::GenerateSha1(const std::string &p_stringToHash, const std::string &p_salt, Format p_format)
    {
      SHA_CTX c;
      unsigned char hash[SHA_DIGEST_LENGTH];

      SHA1_Init(&c);
      SHA1_Update(&c, p_stringToHash.c_str(), p_stringToHash.size());
      SHA1_Update(&c, p_salt.c_str(), p_salt.size());
      SHA1_Final(hash, &c);
      if (p_format == Hexadecimal)
        return (GetHexRepresentation(hash, SHA_DIGEST_LENGTH));
      else if (p_format == Base64)
      {
        std::vector<unsigned char> vect(hash, hash + SHA_DIGEST_LENGTH);
        return (Converter::Base64Encode(vect));
      }
      return ("");
    }

  }
}
