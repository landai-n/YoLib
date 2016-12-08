#ifndef		YO_LOG_H
# define	YO_LOG_H

# include	<string>
# include	<mutex>

#define WRITE_ERROR(x) Yo::Utils::Log::writeError(__func__, x)

namespace Yo
{
  namespace Utils
  {
    class Log
    {
    public:
      static bool writeError(const std::string &, const std::string &);
      static bool writeError(const std::string &);
      static void writeInfo(const std::string &, bool p_verbose = false);
      static void writeWarning(const std::string &);
      static void writeDebug(const std::string &, bool p_cache = false);
      static void Flush();
      static bool Verbose;
    private:
      static std::mutex lock;
      static std::string cache;
    };
  }
}

#endif
