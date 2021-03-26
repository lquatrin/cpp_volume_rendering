#ifndef APPLICATION_USING_GLFW
#define APPLICATION_USING_GLFW

#include "defines.h"

#ifdef USING_GLFW
class GLFWwindow;

class ApplicationGLFW
{
public:
  static void glfw_error_callback (int error, const char* description);

  static void glfwSwapBuffer (void* data);
  static void s_MouseButtonCallback (GLFWwindow* window, int button, int action, int mods);
  static void s_ScrollCallback (GLFWwindow* window, double xoffset, double yoffset);
  static void s_KeyCallback (GLFWwindow* window, int key, int scancode, int action, int mods);
  static void s_CharCallback (GLFWwindow* window, unsigned int codepoint);
  static void s_MouseMotionCallback (GLFWwindow* window, double xpos, double ypos);
  static void s_WindowSizeCallback (GLFWwindow* window, int w, int h);
  
  ApplicationGLFW ();
  ~ApplicationGLFW ();

  bool Init (int argc, char** argv);
  bool InitImGui ();
  void MainLoop ();
  void ImGuiDestroy ();
  void Destroy ();

  const char* GetImGuiglslversion ();

  GLFWwindow* GetWindow ()
  {
    return window;
  }

protected:

private:
  GLFWwindow* window;
  bool use_vsync;
};
#endif

#endif