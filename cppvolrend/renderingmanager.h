/**
 * Rendering Manager for Volume Rendering
 *
 * https://www.cg.tuwien.ac.at/research/vis/datasets/
**/
#ifndef RENDERING_MANAGER_H
#define RENDERING_MANAGER_H
#include "defines.h"

#include <gl_utils/arrayobject.h>
#include <gl_utils/pipelineshader.h>

#include <vis_utils/camera.h>

#include <glm/glm.hpp>
#include <vector>

#include <volvis_utils/datamanager.h>
#include <volvis_utils/renderingparameters.h>

#include <volvis_utils/camerastatelist.h>
#include <volvis_utils/lightsourcelist.h>

class BaseVolumeRenderer;

class RenderingManager
{
public:
  typedef void(*SwapBufferFunc) (void* data);
  SwapBufferFunc f_swapbuffer;
  void* d_swapbuffer;

  typedef void(*RenderFunc) (void* data);
  RenderFunc f_render[4];

  /*! Returns the current instance of Viewer (lazy instantiation).
  */
  static RenderingManager *Instance ();

  /*! Verify if already exists an instance of the Viewer.
  \return exist or not exist (true or false)
  */
  static bool Exists ();

  /*! Just Destroy the instance of the singleton.
  */
  static void DestroyInstance ();

  void InitGL ();
  void InitData ();

  void AddVolumeRenderer (BaseVolumeRenderer* volrend);

  void Display ();
  void Reshape (int w, int h);
  void Keyboard (unsigned char key, int x, int y);
  void KeyboardUp (unsigned char key, int x, int y);
  void MouseButton (int bt, int st, int x, int y);
  void MouseMotion (int x, int y);
  void CloseFunc ();
  void IdleFunc ();
  void PostRedisplay ();

  // Update the volume renderer with the current volume and transfer function
  void UpdateDataAndResetCurrentVRMode ();

  unsigned int GetScreenWidth ()
  {
    return curr_rdr_parameters.GetScreenWidth();
  }

  unsigned int GetScreenHeight ()
  {
    return curr_rdr_parameters.GetScreenHeight();
  }

  bool IdleRendering ()
  {
    return m_idle_rendering;
  }

protected:


private:
  void SaveScreenshot ();
  void UpdateLightSourceCameraVectors ();
  void ResetGLStateConfig ();

  std::string AddAbreviationName (std::string filename, std::string extension = ".png");
  bool GenerateImgFile (std::string out_str, int w, int h, unsigned char *gl_data, bool alpha = true, std::string image_type = "PNG");

  GLubyte* GetFrontBufferPixelData (bool alpha = true);

  // Go to previous renderer
  bool PreviousRenderer ();
  // Go to next renderer
  bool NextRenderer ();
  // Set current volume renderer
  void SetCurrentVolumeRenderer ();

  int m_current_vr_method_id;

  BaseVolumeRenderer* curr_vol_renderer;
  std::vector<BaseVolumeRenderer*> m_vtr_vr_methods;
  std::vector<std::string> m_vtr_vr_ui_names;
  vis::RenderingParameters curr_rdr_parameters;

  std::vector<std::string> m_std_cam_state_names;
  int m_current_camera_state_id;
  vis::CameraStateList m_camera_state_list;

  std::vector<std::string> m_std_lsource_names;
  int m_current_lightsource_data_id;
  vis::LightSourceList m_light_source_list;

  vis::DataManager m_data_mgr;

  bool animate_camera_rotation;

  void SetImGuiInterface ();
  void DrawImGuiInterface ();

  void UpdateFrameRate ();
  
  bool m_imgui_render_ui;

  bool m_imgui_render_manager;
  bool m_idle_rendering;
  double m_ts_current_time;
  double m_ts_last_time;
  int m_ts_n_frames;
  double m_ts_window_fps;
  double m_ts_window_ms;

  bool m_imgui_data_window;

  bool m_imgui_renderer_window;

  std::vector<glm::vec4> s_ref_image;

  static void SingleSampleRender (void* data);
  static void MultiSampleRender (void* data);
  static void DownScalingRender (void* data);
  static void UpScalingRender (void* data);

  /*! Constructor.*/
  RenderingManager ();
  /*! Destructor.*/
  virtual ~RenderingManager ();
  /*! The pointer to the singleton instance.*/
  static RenderingManager* crr_instance;
};

#endif