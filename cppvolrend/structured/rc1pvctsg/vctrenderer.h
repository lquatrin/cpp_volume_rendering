/**
 * 1-Pass - Ray Casting
 * . Structured Datasets
 * . Voxel Cone Tracing for Shadow Generation
 *
 * "Single-GPU" implementation of 2016 paper:
 * . Parallel Distributed, GPU-Accelerated, Advanced Lighting Calculations for Large-Scale Volume Visualization
 * . Min Shih, Silvio Rizzi, Joseph A. Insley, Thomas D. Uram, Venkatram Vishwanath, Mark Hereld, Michael E. Papka, Kwan-Liu Ma
 * . IEEE 6th Symposium on Large Data Analysis and Visualization (LDAV) (2016)
 * . IEEE Xplore Digita Library: https://ieeexplore.ieee.org/document/7156383
 * . DOI: 10.1109/LDAV.2016.7874309
 *
 * Obs: In the paper, the authors consider the aperture angle of the whole angle.
 *      We consider as the "half angle" aperture.
 *
 *  \    |    /
 *   \---|---/ cone apex angle by the authors
 *    \  |  / 
 *     \-| / cone angle considering half angle (our implementation)
 *      \|/
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef VOXEL_CONE_TRACING_GAUSSIAN_PRE_FILTERED_SINGLE_GPU_RAY_CASTING_H
#define VOXEL_CONE_TRACING_GAUSSIAN_PRE_FILTERED_SINGLE_GPU_RAY_CASTING_H

#include <volvis_utils/reader.h>
#include <volvis_utils/utils.h>

#include <gl_utils/arrayobject.h>
#include <gl_utils/bufferobject.h>
#include <gl_utils/texture1d.h>
#include <gl_utils/texture3d.h>

#include <gl_utils/computeshader.h>

#include "preprocessingstages.h"

#include "../../volrenderbase.h"
#include "../../utils/preillumination.h"

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"

class RC1PVoxelConeTracingSGPU : public BaseVolumeRenderer
{
public:
  RC1PVoxelConeTracingSGPU ();
  virtual ~RC1PVoxelConeTracingSGPU ();

  //////////////////////////////////////////////////////////////////////////////
  // BaseVolumeRenderer
  virtual const char* GetName () { return "1-Pass - Ray Casting - Voxel Cone Tracing"; }
  virtual const char* GetAbbreviationName () { return "s_1rc_vct"; }

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

  // rendering shader
  gl::ComputeShader*  cp_shader_rendering;

private:
  void CreateRenderingPass ();
  void DestroyRenderingShaders ();

  gl::Texture1D* m_glsl_transfer_function;

  float m_u_step_size;

  bool m_apply_gradient_shading;

  //////////////////////////////////////////
  // Preprocessing class 
  // . Compute supervoxels and pre integration table
  VCTPreProcessing pre_processing;

  //////////////////////////////////////////
  // Lighting Parameters
  bool apply_ambient_occlusion;

  bool apply_voxel_cone_tracing;
  float cone_step_size;
  float cone_step_size_increase_rate;
  float cone_initial_step;
  float cone_apex_angle;           // Shadow softness
  bool apply_correction_factor;
  float opacity_correction_factor; // Shadow darkness
  int cone_number_of_samples;
  float covered_distance;
};

#endif
