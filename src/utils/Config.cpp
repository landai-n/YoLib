/*!
* \file Config.cpp
* \author Nathanael Landais
*/

#include	<fstream>
#include	"Log.h"
#include	"Config.h"
#include  <unistd.h>

namespace Yo
{
  namespace Utils
  {
    bool Config::Load(const std::string &p_path)
    {
      std::fstream file;
      std::string line;
      std::string currentSection;
      char cwd[1024];

      file.open(p_path.c_str(), std::fstream::in);
      if (getcwd(cwd, sizeof(cwd)) != NULL)
        Log::writeInfo("Open config file at " + std::string(cwd) + "/" + p_path, true);
      if (!file)
      {
        Log::writeError("Config::Load()", "Unable to open " + p_path);
        return (false);
      }
      while (std::getline(file, line))
      {
        std::istringstream token_stream(line);

        token_stream >> line >> std::ws;			// remove whitespaces
        if (line[0] == COMMENT_CHAR || line == "")		// ignore comments and void lines
          continue;
        if (line[0] == OPEN_SECTION_CHAR && line[line.size() - 1] == CLOSE_SECTION_CHAR) // if is new section
        {
          currentSection = line.substr(1, line.size() - 2);
        }
        else
        {
          std::size_t pos;

          if (currentSection == "")
          {
            file.close();
            return (false);
          }
          else if ((pos = line.find("=")) == std::string::npos)
          {
            file.close();
            return (false);
          }
          m_storage[currentSection][line.substr(0, pos)] = line.substr(pos + 1);
        }
      }
      file.close();
      return (true);
    }

  }
}
