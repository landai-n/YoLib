/*!
* \file Zip.h
* \author Nathanael Landais
*/

#ifndef		YO_ZIP_H
# define	YO_ZIP_H

# include	<memory.h>
# include	<zlib.h>

namespace	Yo
{
  namespace Utils
  {
    class Zip
    {
    public:
      static char *Compress(const char *p_buffer, uLongf size, uLongf &p_compressedSize)
      {
        p_compressedSize = ::compressBound(size);
        char *compressed = new char[p_compressedSize];
        ::compress((Bytef *) compressed, &p_compressedSize, (Bytef *) p_buffer, size);
        return (compressed);
      }

      static char *Uncompress(const char *p_buffer, uLongf size, uLongf &p_uncompressedSize)
      {
        char *uncompressed = new char[p_uncompressedSize];
        ::uncompress((Bytef *) uncompressed, &p_uncompressedSize, (Bytef *) p_buffer, size);
        return (uncompressed);
      }
    };
  }
}

#endif
