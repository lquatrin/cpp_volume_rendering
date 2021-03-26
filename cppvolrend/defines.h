#pragma once

//#define UPDATE_WINDOW_TITLE

// Using suggestion on using MAKE_STR macro at C++ code from "naoyam"
// . https://github.com/LLNL/lbann/issues/117
#define MAKE_STR(x) _MAKE_STR(x)
#define _MAKE_STR(x) #x

#define CPPVOLREND_DIR MAKE_STR(CMAKE_PATH_TO_APP_FOLDER)
#define CPPVOLREND_SHADER_DIR MAKE_STR(CMAKE_PATH_TO_APP_FOLDER)"shader/"
#define CPPVOLREND_INCLUDE_DIR MAKE_STR(CMAKE_PATH_TO_INCLUDE)
#define CPPVOLREND_DATA_DIR MAKE_STR(CMAKE_PATH_TO_DATA_FOLDER)

#define MULTISAMPLE_AVAILABLE
#define MULTISAMPLE_NUMBEROFSAMPLES_W 2
#define MULTISAMPLE_NUMBEROFSAMPLES_H 2

// -> DEFINED IN CMAKE
//----------------------------------------------------------------------
// must link "x64/glfw/[release|debug]/glfw3.lib"
//#define USING_GLFW 
// --> COMMENT USING_FREEGLUT TO USE GLFW
// http://freeglut.sourceforge.net/
#define USING_FREEGLUT
//----------------------------------------------------------------------

// undefine glfw if freeglut is defined
#ifdef USING_FREEGLUT
#undef USING_GLFW
#endif

#include <GL/glew.h>

#ifdef USING_GLFW
//#define USING_VULKAN
#endif

#ifdef USING_VULKAN
#include <C:/VulkanSDK/1.1.114.0/Include/vulkan/vulkan.h>
#endif

#ifdef USING_FREEGLUT
#include <GL/freeglut.h>
#else
#ifdef USING_GLFW
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#endif
#endif

// https://github.com/ocornut/imgui
// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed
#endif

#include "imgui.h"

#ifdef USING_FREEGLUT
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"
#else
 #ifdef USING_GLFW
   #include "imgui_impl_glfw.h"
   #include "imgui_impl_opengl3.h"
 
   // About OpenGL function loaders: modern OpenGL doesn't have a standard header file and requires individual function pointers to be loaded manually.
   // Helper libraries are often used for this purpose! Here we are supporting a few common ones: gl3w, glew, glad.
   // You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
   #if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
   #include <GL/gl3w.h>    // Initialize with gl3wInit()
   #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
   #include <GL/glew.h>    // Initialize with glewInit()
   #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
   #include <glad/glad.h>  // Initialize with gladLoadGL()
   #else
   #include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
   #endif
 
   // Include glfw3.h after our OpenGL definitions
   #include <GLFW/glfw3.h>
 #endif
#endif