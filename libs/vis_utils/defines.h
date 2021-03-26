#ifndef VIS_UTILS_DEFINES_H
#define VIS_UTILS_DEFINES_H

// Using suggestion on using MAKE_STR macro at C++ code from "naoyam"
// . https://github.com/LLNL/lbann/issues/117
#ifndef MAKE_STR
#define MAKE_STR(x) _MAKE_STR(x)
#endif

#ifndef _MAKE_STR
#define _MAKE_STR(x) #x
#endif

#include <iostream>
#include <string>

namespace vis
{

class Utils
{
public:
  static std::string GetShaderPath ();
  static std::string GetResourcesRepositoryPath ();
};

}

#endif