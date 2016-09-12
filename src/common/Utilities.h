/*=========================================================================

  Program:  Birch (A simple image viewer)
  Module:   Utilities.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
//
// .SECTION Description
// A utilities header to include typedefs, macros, global functions, etc.
//

#ifndef __Utilities_h
#define __Utilities_h

#define BIRCH_VERSION_MAJOR @BIRCH_VERSION_MAJOR@
#define BIRCH_VERSION_MINOR @BIRCH_VERSION_MINOR@
#define BIRCH_VERSION_PATCH @BIRCH_VERSION_PATCH@

#define BIRCH_API_DIR "@BIRCH_API_DIR@"
#define BIRCH_APP_DIR "@BIRCH_APP_DIR@"
#define BIRCH_QT_DIR "@BIRCH_QT_DIR@"
#define BIRCH_VTK_DIR "@BIRCH_VTK_DIR@"

// C includes
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// C++ includes
#include <algorithm>
#include <cctype>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

/**
 * @addtogroup Birch
 * @{
 */

namespace Birch
{
  class Utilities
  {
  public:
    inline static std::string exec(const char* command)
    {
      FILE* pipe = popen(command, "r");
      if (!pipe) return "ERROR";
      char buffer[128];
      std::string result = "";
      while (!feof(pipe))
        if (fgets(buffer, 128, pipe) != NULL)
          result += buffer;
      pclose(pipe);
      return result;
    }

    inline static std::string getTime(std::string format)
    {
      char buffer[256];
      time_t rawtime;
      time(&rawtime);
      strftime(buffer, 256, format.c_str(), localtime(&rawtime));
      return std::string(buffer);
    }

    inline static std::string toLower(std::string str)
    {
      std::string returnString = str;
      std::transform(str.begin(), str.end(), returnString.begin(), tolower);
      return returnString;
    }

    inline static std::string toUpper(std::string str)
    {
      std::string returnString = str;
      std::transform(str.begin(), str.end(), returnString.begin(), toupper);
      return returnString;
    }

    inline static bool fileExists(std::string filename)
    {
      if (filename.empty()) return false;
      return access(filename.c_str(), R_OK)  == 0;
    }

    inline static std::string getFileExtension(std::string filename)
    {
      std::string::size_type dot_pos = filename.rfind(".");
      std::string extension = (dot_pos == std::string::npos) ? "" :
        filename.substr(dot_pos);
      return extension;
    }

    inline static std::string getFilenamePath(std::string filename)
    {
      std::string::size_type slash_pos = filename.rfind("/");
      if (slash_pos != std::string::npos)
      {
        std::string path = filename.substr(0, slash_pos);
        if (path.size() == 2 && path[1] == ':')
        {
          return path + '/';
        }
        if (path.size() == 0)
        {
          return "/";
        }
        return path;
      }
      else
      {
        return "";
      }
    }

    inline static std::string getFilenameName(std::string filename)
    {
      std::string::size_type slash_pos = filename.find_last_of("/");
      if (slash_pos != std::string::npos)
      {
        return filename.substr(slash_pos + 1);
      }
      else
      {
        return filename;
      }
    }

    /**
     * Divides a string by the provided separator, returning the results as a vector of strings
     */
    inline static std::vector< std::string > explode(
      std::string str, std::string separator)
    {
      std::vector< std::string > results;
      int found = str.find_first_of(separator);
      while (found != std::string::npos)
      {
        if (found > 0) results.push_back(str.substr(0, found));
        str = str.substr(found + 1);
        found = str.find_first_of(separator);
      }
      if (str.size() > 0) results.push_back(str);
      return results;
    }

    /**
     * Concatenates a vector of strings by the provided separator, returning the results as string
     */
    inline static std::string implode(
      const std::vector<std::string> &vec, const std::string &separator)
    {
      std::string result;
      for (std::vector<std::string>::const_iterator it = vec.begin();
        it != vec.end(); ++it)
      {
        result += *it;
        if (1 + it != vec.end()) result += separator;
      }
      return result;
    }

    /**
     * Removes all space characters (as defined by std::isspace) from the left side of a string
     */
    inline static std::string &ltrim(std::string &s)
    {
      s.erase(
        s.begin(), std::find_if(
          s.begin(), s.end(), std::not1(
            std::ptr_fun<int, int>(std::isspace))));
      return s;
    }


    /**
     * Removes all space characters (as defined by std::isspace) from the right side of a string
     */
    inline static std::string &rtrim(std::string &s)
    {
      s.erase(
        std::find_if(
          s.rbegin(), s.rend(), std::not1(
            std::ptr_fun<int, int>(std::isspace))).base(), s.end());
      return s;
    }

    /**
     * Removes all space characters (as defined by std::isspace) from both sides of a string
     */
    inline static std::string &trim(std::string &s)
    {
      return ltrim(rtrim(s));
    }

    /**
     * Retain n characters from the left side of a string
     */
    inline static void left(std::string &s, const int &n)
    {
      if (0 > n) return;
      size_t pos = n;
      s.erase(pos, std::string::npos);
    }

    /**
     * Retain n characters from the right side of a string
     */
    inline static void right(std::string &s, const int &n)
    {
      if (0 > n) return;
      size_t pos = 0;
      size_t len = s.size();
      len = n >= len ? std::string::npos : len - n;
      s.erase(pos, len);
    }
  };
}  // namespace Birch

#endif  // __Utilities_h
