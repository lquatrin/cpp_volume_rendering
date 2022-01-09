/**
 * 1-Pass - Ray Casting
 * . Structured Datasets
 * . Ambient Occlusion and Shadows using Summed Area Table 3D using extinction coefficients

 * Implementation of 2011 paper:
 * . Extinction-Based Shading and Illumination in GPU Volume Ray-Casting
 * . Philipp Schlegel, Maxim Makhinya, and Renato Pajarola
 * . IEEE TRANSACTIONS ON VISUALIZATION AND COMPUTER GRAPHICS (2011, Vol. 17, No. 12)
 * . IEEE Xplore Digital Library: https://ieeexplore.ieee.org/document/6064942
 * . DOI: 10.1109/TVCG.2011.198
 * . Videos: https://www.youtube.com/watch?v=Pb_qGiksDJ4
 *
 * Missing:
 * . Ambient Occlusion with Color Bleeding
 *
 * Obs: In the paper, the authors consider the aperture angle of the whole angle.
 *      We consider as the "half angle" aperture.
 *
 * Useful links aboud Summed-Area Tables:
 * . GPU-Efficient Recursive Filtering and Summed-Area Tables: http://w3.impa.br/~diego/publications/NehEtAl11.pdf
 * . Parallel Algorithms for the Summed Area Table on the Asynchronous Hierarchical Memory Machine, with GPU implementations: http://www.cs.hiroshima-u.ac.jp/cs/_media/sat-icpp.pdf
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef SINGLE_PASS_VOLUME_RENDERING_RAY_CASTING_EXTINCTION_BASED_SHADING_AND_ILLUMINATION_IN_GPU_VOLUME_RAY_CASTING_H
#define SINGLE_PASS_VOLUME_RENDERING_RAY_CASTING_EXTINCTION_BASED_SHADING_AND_ILLUMINATION_IN_GPU_VOLUME_RAY_CASTING_H

#include <gl_utils/texture1d.h>
#include <gl_utils/texture3d.h>

#include <gl_utils/arrayobject.h>
#include <gl_utils/bufferobject.h>

#include <gl_utils/computeshader.h>

#include "../../volrenderbase.h"
#include "../../utils/preillumination.h"

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"

class RC1PExtinctionBasedShading : public BaseVolumeRenderer
{
public:
  RC1PExtinctionBasedShading ();
  virtual ~RC1PExtinctionBasedShading ();

  //////////////////////////////////////////////////////////////////////////////
  // BaseVolumeRenderer
  virtual const char* GetName () { return "1-Pass - Ray Casting - Extinction-based"; }
  virtual const char* GetAbbreviationName () { return "s_1rc_eb"; }

  virtual void Clean ();
  virtual void ReloadShaders ();

  virtual bool Init (int shader_width, int shader_height);
  virtual bool Update (vis::Camera* camera);
  virtual void Redraw ();
  virtual void MultiSampleRedraw ();
  virtual void DownScalingRedraw ();
  virtual void UpScalingRedraw ();
  
  virtual void SetImGuiComponents ();
  virtual void FillParameterSpace(ParameterSpace& pspace) override;

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
  
  // Summed Area Table 3D using Extinction Coefficients
  int st_w, st_h, st_d;
  gl::Texture3D* glsl_sat3d_tex;

  // Rendering shaders
  gl::ComputeShader* cp_shader_rendering;

  glm::mat4 ProjectionMatrix, ViewMatrix;
  void CreateRenderingPass ();

private:
  void DestroyRenderingShaders ();
  void DestroySummedAreaTable ();

  gl::Texture3D* GenerateExtinctionSAT3DTex (vis::StructuredGridVolume* vol, vis::TransferFunction* tf);

  gl::Texture1D* m_glsl_transfer_function;

  float m_u_step_size;

  bool m_apply_gradient_shading;
  
  bool transfer_function_changed;
  
  //////////////////////////////////////////
  // Lighting Parameters
  bool apply_ambient_occlusion;
  int ambient_occlusion_shells;
  float ambient_occlusion_radius;
 
  bool apply_directional_shadows;
  int dir_shadow_cone_samples;
  float dir_shadow_cone_angle;
  float dir_shadow_sample_interval;
  float dir_shadow_initial_step;
  float dir_shadow_user_interface_weight;
  float dir_cone_max_distance;
  int type_of_shadow;
};

#endif