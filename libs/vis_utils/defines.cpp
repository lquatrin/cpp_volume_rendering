#include "defines.h"

namespace vis
{
std::string Utils::GetShaderPath ()
{
  std::string s_path = MAKE_STR(CMAKE_VIS_UTILS_PATH_TO_SHADER)"";
  return s_path;
}

std::string Utils::GetResourcesRepositoryPath ()
{
  std::string s_path = MAKE_STR(CMAKE_PATH_TO_RESOURCES)"";
  return s_path;
}

}
