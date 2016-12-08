/*!
* \file ConfigManager.h
* \author Nathanael Landais
*/

#ifndef		YO_CONFIGMANAGER_H
# define	YO_CONFIGMANAGER_H

# include	<map>
# include	<string>
# include	"Config.h"

namespace Yo
{
  namespace Utils
  {
    class ConfigManager
    {
    public:
      static Config &GetConfig(const std::string &);
    private:
      static std::map<std::string, Config> m_storage;
    };
  }
}

#endif
