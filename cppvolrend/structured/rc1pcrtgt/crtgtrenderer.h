/**
 * 1-Pass - Ray Casting
 * . Structured Datasets
 * . Directional Ambient Occlusion and Shadows tracing rays
 * . used as our "Ground Truth"
 *
 * Cone Occlusion Ground Truth Overview:
 * . We cast and evaluate rays from each cone.
 * . Each occlusion ray is weighted by the cosine of angle (dot product between occlusion ray and "normal").
 *
 * We must change the current regedit variables on Windows to run shaders that takes some seconds to run
 * . TdrLevel     - 0     (3 def)
 * . TdrDelay     - 10000 (2 def)
 * . TdrLimitTime - 10000 (60 def)
 * . TdrDdiDelay  - 10000 (optional)
 * . Windows must be restarted to apply new settings.
 *
 * Useful links about gpu timeout:
 * . https://docs.microsoft.com/en-us/windows-hardware/drivers/display/tdr-registry-keys
 * . https://www.khronos.org/assets/uploads/developers/library/2014-siggraph-bof/KITE-BOF_Aug14.pdf
 * . https://www.pugetsystems.com/labs/hpc/Working-around-TDR-in-Windows-for-a-better-GPU-computing-experience-777/
 * . https://www.reddit.com/r/opengl/comments/3c4ad4/how_to_prevent_hitting_tdr_timeout_with_long/
 * . https://www.opengl.org/discussion_boards/showthread.php/170546-Intensive-Shaders-%28-1-second-per-primitive%29
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef GROUND_TRUTH_RAY_CASTING_CONE_LIGHT_AMBIENT_AND_SHADOWS_STEPS_H
#define GROUND_TRUTH_RAY_CASTING_CONE_LIGHT_AMBIENT_AND_SHADOWS_STEPS_H

#include <gl_utils/texture1d.h>
#include <gl_utils/texture2d.h>
#include <gl_utils/texture3d.h>

#include <gl_utils/arrayobject.h>
#include <gl_utils/bufferobject.h>

#include <gl_utils/pipelineshader.h>
#include <gl_utils/computeshader.h>

#include "../../volrenderbase.h"
#include <gl_utils/framebufferobject.h>

#include <queue>

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"

class RC1PConeLightGroundTruthSteps : public BaseVolumeRenderer
{
public:
  RC1PConeLightGroundTruthSteps ();
  virtual ~RC1PConeLightGroundTruthSteps ();

  virtual const char* GetName () { return "1-Pass - Ray Casting - Cone Ground Truth (Steps)"; }
  virtual const char* GetAbbreviationName () { return "s_1rc_gt_c"; }

  virtual void Clean ();
  virtual void ReloadShaders ();

  virtual bool Init (int shader_width, int shader_height);
  virtual bool Update (vis::Camera* camera);
  void PreRedraw ();
  void RedrawFrameTexture ();
  void RedrawCube ();
  virtual void Redraw ();
  virtual void MultiSampleRedraw ();
  virtual void DownScalingRedraw ();
  virtual void UpScalingRedraw ();
  
  virtual void Reshape (int w, int h);

  virtual vis::GRID_VOLUME_DATA_TYPE GetDataTypeSupport ()
  {
    return vis::GRID_VOLUME_DATA_TYPE::STRUCTURED;
  }

  virtual void SetImGuiComponents();

protected:
  gl::ComputeShader* m_gt_rendering;
  GLfloat* m_rgba_screen_data;
  GLfloat* m_rgba_state_fragment;
  gl::ComputeShader* m_cp_rendering;
  gl::Texture1D* m_tex_transfer_function;
  gl::Texture2D* m_tex_gt_state;

  bool m_frame_outdated;
  bool m_show_frame_texture;

  float m_u_step_size;

  bool m_apply_gradient;

  float m_u_light_ray_initial_step;
  float m_u_light_ray_step_size;

  bool m_light_parameters_outdated;

  bool m_apply_occlusion;
  int m_occ_num_rays_sampled;
  float m_occ_cone_aperture_angle;
  gl::Texture1D* m_occ_tex_raysampled_vectors;
  float m_occ_cone_distance_eval;

  bool m_apply_shadows;
  int m_sdw_num_rays_sampled;
  float m_sdw_cone_aperture_angle;
  gl::Texture1D* m_sdw_tex_raysampled_vectors;
  float m_sdw_cone_distance_eval;
  int m_shadow_type;

private:
  void CreateIntegrationPass ();
  void DestroyIntegrationPass ();
  void ResetOutputFrameGeneration ();
};

#endif