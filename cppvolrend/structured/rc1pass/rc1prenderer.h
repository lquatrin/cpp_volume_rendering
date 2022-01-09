/**
 * 1-Pass - Ray Casting
 * . Structured Datasets
 * . Faster when implemented with Compute Shader:
 *   . A Comparison between GPU-based Volume Ray Casting Implementations:
 *     Fragment Shader, Compute Shader, OpenCL, and CUDA
 *   . Francisco Sans, Rhadamés Carmona
 *   . CLEI Electronic Journal, Volume 20, Number 2, Paper 7, 2017
 *   . DOI: 10.19153/cleiej.20.2.7
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef SINGLE_PASS_VOLUME_RENDERING_RAY_CASTING_H
#define SINGLE_PASS_VOLUME_RENDERING_RAY_CASTING_H

#include <gl_utils/texture1d.h>
#include <gl_utils/texture2d.h>
#include <gl_utils/texture3d.h>

#include <gl_utils/arrayobject.h>
#include <gl_utils/bufferobject.h>

#include <gl_utils/computeshader.h>

#include "../../volrenderbase.h"

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"

class RayCasting1Pass : public BaseVolumeRenderer
{
public:
  RayCasting1Pass ();
  virtual ~RayCasting1Pass ();

  //////////////////////////////////////////
  // Virtual base functions
  virtual const char* GetName () { return "1-Pass - Ray Casting"; }
  virtual const char* GetAbbreviationName () { return "s_1rc"; }

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

  virtual void FillParameterSpace(ParameterSpace& pspace) override;

  float m_u_step_size;

protected:

private:
  void CreateRenderingPass ();
  void DestroyRenderingPass ();
  void RecreateRenderingPass ();
  
  gl::Texture1D* m_glsl_transfer_function;

  gl::ComputeShader*  cp_shader_rendering;


  bool m_apply_gradient_shading;
  
};

#endif