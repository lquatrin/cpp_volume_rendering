#ifndef APPLICATION_USING_FREEGLUT
#define APPLICATION_USING_FREEGLUT

#include "defines.h"

#ifdef USING_FREEGLUT
class ApplicationFreeGLUT
{
public:
  static void glutSwapBuffer (void* data);
  static void Display (void);
  static void s_Reshape (int w, int h);
  static void s_Keyboard (unsigned char key, int x, int y);
  static void s_KeyboardUp (unsigned char c, int x, int y);
  static void s_OnMouse (int glut_button, int state, int x, int y);
  static void s_OnMotion (int x, int y);
  static void s_MouseWheel (int wheel, int direction, int x, int y);
  static void s_SpecialFunc (int key, int x, int y);
  static void s_SpecialUpFunc (int key, int x, int y);
  static void s_PassiveMotionFunc (int x, int y);
  static void s_CloseFunc ();
  static void s_IdleFunc ();
  
  ApplicationFreeGLUT ();
  ~ApplicationFreeGLUT ();

  bool Init (int argc, char** argv);
  bool InitImGui ();
  void MainLoop ();
  void ImGuiDestroy ();
  void Destroy ();

protected:

private:
};
#endif

#endif