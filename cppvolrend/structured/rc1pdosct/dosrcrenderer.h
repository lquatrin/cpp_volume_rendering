/**
 * 1-Pass - Ray Casting - Dir. Occlusion Shading
 * . Structured Datasets
 * . Directional Ambient Occlusion and Shadows
 * . Gaussian filters applied in the opacity field
 *
 * Implementation of 2019 paper:
 * . Interactive directional ambient occlusion and shadow computations for volume ray casting
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef SINGLE_PASS_VOLUME_RENDERING_RAY_CASTING_CONE_TRACING_DIRECTIONAL_OCCLUSION_SHADING_H
#define SINGLE_PASS_VOLUME_RENDERING_RAY_CASTING_CONE_TRACING_DIRECTIONAL_OCCLUSION_SHADING_H

#include <gl_utils/texture1d.h>
#include <gl_utils/texture2d.h>
#include <gl_utils/texture3d.h>

#include <gl_utils/arrayobject.h>
#include <gl_utils/bufferobject.h>

#include <gl_utils/pipelineshader.h>
#include <gl_utils/framebufferobject.h>

#include <gl_utils/computeshader.h>

#include "../../volrenderbase.h"
#include "../../utils/preillumination.h"

#include "extcoefvolumegenerator.h"
#include "conegaussiansampler.h"

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"

class RC1PConeTracingDirOcclusionShading : public BaseVolumeRenderer
{
public:
  RC1PConeTracingDirOcclusionShading ();
  virtual ~RC1PConeTracingDirOcclusionShading ();

  //////////////////////////////////////////////////////////////////////////////
  // Virtual functions
  virtual const char* GetName () { return "1-Pass - Ray Casting - Dir. Occlusion Shading"; }
  virtual const char* GetAbbreviationName () { return "s_1rc_dos"; }

  virtual void Clean ();
  virtual void ReloadShaders ();

  virtual bool Init (int shader_width, int shader_height);
  virtual bool Update (vis::Camera* camera);
  virtual void Redraw ();
  virtual void MultiSampleRedraw ();
  virtual void DownScalingRedraw ();
  virtual void UpScalingRedraw ();
  
  virtual void SetImGuiComponents ();

  virtual vis::GRID_VOLUME_DATA_TYPE GetDataTypeSupport ()
  {
    return vis::GRID_VOLUME_DATA_TYPE::STRUCTURED;
  }

protected:
  //////////////////////////////////////////
  // Light Computation Mode
  PreIlluminationStructuredVolume m_pre_illum_str_vol;
  gl::ComputeShader* cp_lightcache_shader;
  virtual void PreComputeLightCache (vis::Camera* camera);

  gl::ComputeShader* cp_shader_rendering;

  //////////////////////////////////////////
  // Extinction Coefficient Volume
  ExtinctionCoefficientVolume ext_coef_vol_gen;
  gl::Texture3D* glsl_ext_coef_volume;
  double time_vol_generator;

  //////////////////////////////////////////
  // Cone Lighting Parameters
  bool glsl_apply_occlusion;
  gl::Texture1D* glsl_occ_sectionsinfo;
  ConeGaussianSampler sampler_occlusion;

  bool glsl_apply_shadow;
  gl::Texture1D* glsl_sdw_sectionsinfo;
  ConeGaussianSampler sampler_shadow;
  int type_of_shadow;

private:
  void CreateRenderingPass ();
  void DestroyRenderingPass ();

  void GenerateExtCoefVolume ();
  void DestroyExtCoefVolume ();

  void GenerateConeSamples ();
  void DestroyConeSamples ();
   
  // Extinction coefficient bindings
  bool bind_volume_of_gaussians;
  void BindExtinctionCoefficientVolume ();

  // Cone Occlusion bindings
  bool bind_cone_occlusion_vars;
  void BindConeOcclusionUniforms ();

  // Cone Shadow bindings
  bool bind_cone_shadow_vars;
  void BindConeShadowUniforms ();

  void DestroyPreIlluminationShader ();

  gl::Texture1D* m_glsl_transfer_function;

  float m_u_step_size;

  bool m_apply_gradient_shading;
};

#endif