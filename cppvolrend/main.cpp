/**
 * C++ Volume Rendering Application
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#include "defines.h"
#include "renderingmanager.h"
#include "volrenderbase.h"

#include <cstdio>
#include <fstream>
#include <iostream>

#include <glm/glm.hpp>

#include <math_utils/utils.h>
#include <glm/gtc/type_ptr.hpp>

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
#include "volrendernull.h"
// 1-pass - Ray Casting - GLSL
#include "structured/rc1pass/rc1prenderer.h"
#include "structured/rc1pisoadapt/rc1pisoadaptrenderer.h"
#include "structured/rc1pcrtgt/crtgtrenderer.h"
#include "structured/rc1pdosct/dosrcrenderer.h"
#include "structured/rc1pextbsd/ebsrenderer.h"
#include "structured/rc1pvctsg/vctrenderer.h"
// Slice based
#include "structured/sbtmdos/sbtmdosrenderer.h"
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

#ifdef USING_FREEGLUT
#include "app_freeglut.h"
ApplicationFreeGLUT app;
#else
#ifdef USING_GLFW
#include "app_glfw.h"
ApplicationGLFW app;
#endif
#endif


float k (float x) {
  x = abs(x);
  return x > 1.f ? 0.0f : 1.0f - x;
}

int main (int argc, char **argv)
{
  if (!app.Init(argc, argv)) return 1;

  RenderingManager::Instance()->InitGL();

  // Adding the rendering modes
  //-----------------------------------------------------------------------------------------------------------------------------------------------------------------
  RenderingManager::Instance()->AddVolumeRenderer(new NullRenderer());
  //-----------------------------------------------------------------------------------------------------------------------------------------------------------------
  // 1-pass - Ray Casting - GLSL
  RenderingManager::Instance()->AddVolumeRenderer(new RayCasting1Pass());
  RenderingManager::Instance()->AddVolumeRenderer(new RayCasting1PassIsoAdapt());
  RenderingManager::Instance()->AddVolumeRenderer(new RC1PConeLightGroundTruthSteps());
  RenderingManager::Instance()->AddVolumeRenderer(new RC1PConeTracingDirOcclusionShading());
  RenderingManager::Instance()->AddVolumeRenderer(new RC1PExtinctionBasedShading());
  RenderingManager::Instance()->AddVolumeRenderer(new RC1PVoxelConeTracingSGPU());
  //-----------------------------------------------------------------------------------------------------------------------------------------------------------------
  // Slice based
  RenderingManager::Instance()->AddVolumeRenderer(new SBTMDirectionalOcclusionShading());
  //-----------------------------------------------------------------------------------------------------------------------------------------------------------------

  app.InitImGui();
  RenderingManager::Instance()->InitData();

  app.MainLoop();

  app.ImGuiDestroy();
  app.Destroy();

  return 0;
}