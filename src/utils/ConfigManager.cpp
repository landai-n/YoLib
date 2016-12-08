/*!
* \file ConfigManager.cpp
* \author Nathanael Landais
*/

#include	"ConfigManager.h"

namespace Yo
{
  namespace Utils
  {
    std::map<std::string, Config> ConfigManager::m_storage;

    Config &ConfigManager::GetConfig(const std::string &p_configFile)
    {
      if (m_storage.find(p_configFile) == m_storage.end())
        m_storage[p_configFile].Load(p_configFile);
      return (m_storage[p_configFile]);
    }
  }
}
