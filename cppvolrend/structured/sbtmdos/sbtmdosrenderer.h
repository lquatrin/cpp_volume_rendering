/**
 * Slice Texture-based
 * . Directional Occlusion Shading
 *
 * Implementation of 2009 paper:
 * . A Directional Occlusion Shading Model for Interactive Direct Volume Rendering
 * . Mathias Schott, Vincent Pegoraro, Charles D. Hansen, Kevin Boulanger, Kadi Bouatouch
 * . Computer Graphics Forum 28(3):855-862 (2009)
 * . Link: https://cgl.ethz.ch/teaching/scivis_common/Literature/Schott2009.pdf
 * . ResearchGate: 220507521_A_Directional_Occlusion_Shading_Model_for_Interactive_Direct_Volume_Rendering
 * . DOI: 10.1111/j.1467-8659.2009.01464.x
 * . Wiley Online Library: https://onlinelibrary.wiley.com/doi/full/10.1111/j.1467-8659.2009.01464.x
 *
 * Important Parameters:
 * . Slice Resolution: 512 x 512, 16-bit precision floating-point (FBO)
 * . Multiple Render Targets (MRT) buffers:
 *   - Eye Color Accumulation
 *   - Occlusion
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef SLICE_BASED_TEXTURE_MAPPING_VOLUME_RENDERING_DIRECTIONAL_OCCLUSION_SHADING_H
#define SLICE_BASED_TEXTURE_MAPPING_VOLUME_RENDERING_DIRECTIONAL_OCCLUSION_SHADING_H

#include <volvis_utils/reader.h>
#include <volvis_utils/utils.h>

#include <gl_utils/arrayobject.h>
#include <gl_utils/bufferobject.h>
#include <gl_utils/texture1d.h>
#include <gl_utils/texture3d.h>

#include <gl_utils/pipelineshader.h>
#include <gl_utils/framebufferobject.h>
#include "layeredframebufferobject.h"

#include "../../volrenderbase.h"

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"

class SBTMDirectionalOcclusionShading : public BaseVolumeRenderer
{
public:
  SBTMDirectionalOcclusionShading ();
  virtual ~SBTMDirectionalOcclusionShading ();

  //////////////////////////////////////////////////////////////////////////////
  // BaseVolumeRenderer
  virtual const char* GetName () { return "Slice-based - Directional Occlusion"; }
  virtual const char* GetAbbreviationName () { return "s_sbtm_dos"; }

  virtual void Clean ();
  virtual void ReloadShaders ();

  virtual bool Init (int shader_width, int shader_height);
  virtual bool Update (vis::Camera* camera);
  virtual void Redraw ();
  virtual void MultiSampleRedraw ();
  virtual void DownScalingRedraw ();
  virtual void UpScalingRedraw ();

  virtual void Reshape (int w, int h);
  
  virtual void SetImGuiComponents ();

  virtual vis::GRID_VOLUME_DATA_TYPE GetDataTypeSupport ()
  {
    return vis::GRID_VOLUME_DATA_TYPE::STRUCTURED;
  }

  int GetScreenTextureID () override {
    return lfb_object.GetColorAttachmentID(2);
  }

protected:
  gl::PipelineShader* ps_shader_rendering;

  gl::PipelineShader* shader_blend_pass;

  glm::mat4 ProjectionMatrix, ViewMatrix;

  gl::ArrayObject*  quad_vao;
  gl::BufferObject* quad_vbo;
  gl::BufferObject* quad_ibo;

private:
  float m_u_step_size;

  typedef struct SliceQuad
  {
    glm::vec3 pt_lb;
    glm::vec3 tx_lb;

    glm::vec3 pt_rb;
    glm::vec3 tx_rb;

    glm::vec3 pt_rt;
    glm::vec3 tx_rt;

    glm::vec3 pt_lt;
    glm::vec3 tx_lt;

    float pos_from_near;
  } SliceQuad;

  class ProxyGeometry
  {
  public:
    ProxyGeometry() {}
    ~ProxyGeometry() {}
    std::vector<SliceQuad> quads;
  protected:
  private:
  };

  int shader_width;
  int shader_height;

  bool evaluate_dir_occlusion;
  float cone_half_angle;
  glm::vec2 grid_size;
  LayeredFrameBufferObject lfb_object;
  LayeredFrameBufferObject lfb_objects[2];
  unsigned int draw_buffer;

  void CreateSlicePass (vis::StructuredGridVolume* vr_volume);
  void CreateBlendPass ();

  void ComputeProxyGeometry();
  void ComputeMinMaxZ(glm::vec3 aabb);
  void ComputeMinMaxZBoudingBox(glm::vec3 aabb);

  gl::Texture1D* m_glsl_transfer_function;

  glm::vec3 vol_scale;
  float minimum_z;
  float maximum_z;

  glm::vec3 cam_eye;
  glm::vec3 cam_dir;
  glm::vec3 cam_up;
  glm::vec3 cam_right;

  double volume_diagonal;

  int max_number_of_passes;
  int number_of_passes;

  ProxyGeometry proxy_geom;

  float occ_ui_weight_percentage;
};

#endif
