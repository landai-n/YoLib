/*!
* \file Config.h
* \author Nathanael Landais
*/

#ifndef		YO_CONFIG_H
# define	YO_CONFIG_H

# define	COMMENT_CHAR		'#'
# define	OPEN_SECTION_CHAR	'['
# define	CLOSE_SECTION_CHAR	']'

# include	<map>
# include	<sstream>
# include	"Log.h"
# include	"Converter.h"

namespace Yo
{
  namespace Utils
  {
    class 	Config
    {
    public:
      bool		Load(const std::string &);
      void    EraseValue(const std::string &p_sectionName, const std::string &p_varName)
      {
        if (m_storage[p_sectionName].find(p_varName) == m_storage[p_sectionName].end())
          return;
        m_storage[p_sectionName].erase(p_varName);
      }
      inline const std::string &GetValue(const std::string &p_sectionName, const std::string &p_varName)
      {
        if (m_storage[p_sectionName].find(p_varName) == m_storage[p_sectionName].end())
          Log::writeError("Config::Load()", p_sectionName + "::" + p_varName + " not defined");
        return (m_storage[p_sectionName][p_varName]);
      }

      template <typename T>
      inline T GetValue(const std::string &p_sectionName, const std::string &p_varName)
      {
        return (Converter::ParseString<T>(GetValue(p_sectionName, p_varName)));
      }

    private:
      std::map<std::string, std::map<std::string, std::string>> m_storage;
    };
  }
}

#endif
