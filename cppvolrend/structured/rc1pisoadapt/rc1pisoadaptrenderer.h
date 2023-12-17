/**
 * Isosurface raycaster with adaptive step size.
 *
 * @author Tino Weinkauf
**/
#pragma once

#include "../../volrenderbase.h"

class RayCasting1PassIsoAdapt : public BaseVolumeRenderer
{
public:
  RayCasting1PassIsoAdapt();
  virtual ~RayCasting1PassIsoAdapt();
  
  virtual const char* GetName();
  virtual const char* GetAbbreviationName();
  virtual vis::GRID_VOLUME_DATA_TYPE GetDataTypeSupport();

  virtual void Clean();
  virtual bool Init(int shader_width, int shader_height);
  virtual void ReloadShaders();

  virtual bool Update(vis::Camera* camera);
  virtual void Redraw();

  virtual void FillParameterSpace(ParameterSpace& pspace) override;

  virtual void SetImGuiComponents();

protected:
  float m_u_isovalue;

  /// Step size near the isovalue.
  float m_u_step_size_small;

  /// Step size when the data value is not near the isovalue.
  float m_u_step_size_large;

  /// Withing this range around the isovalue, the small step size will be chosen.
  /// Outside of this range, the large step size will be used.
  float m_u_step_size_range;
  
  glm::vec4 m_u_color;
  bool m_apply_gradient_shading;

private:
  gl::ComputeShader* cp_shader_rendering;

};

