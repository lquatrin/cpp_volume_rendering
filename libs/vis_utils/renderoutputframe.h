#ifndef VIS_UTILS_RENDER_OUTPUT_FRAME_H
#define VIS_UTILS_RENDER_OUTPUT_FRAME_H

#include <gl_utils/texture2d.h>
#include <gl_utils/pipelineshader.h>
#include <gl_utils/computeshader.h>

#include <gl_utils/arrayobject.h>
#include <gl_utils/bufferobject.h>

#include <vis_utils/filters/utils.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

namespace vis
{

class RenderFrameToScreen
{
public:
  RenderFrameToScreen (std::string shader_folder);
  ~RenderFrameToScreen ();

  void Clean ();

  void UpdateScreenResolution (int s_w, int s_h);
  void UpdateScreenResolutionMultiScaling (int s_w, int s_h, int mw = 0, int mh = 0);
  int GetWidth ();
  int GetHeight ();

  void ClearShaders ();
  void ClearTextures ();
  void ClearTexture ();
  void ClearTextureImage ();
  void BindImageTexture (bool multisample = false);
  void Draw ();

  void Draw (gl::Texture2D* screen_output);
  void Draw (GLuint screen_output_id);

  void SetMultiResolutionScreenMultiplier (glm::ivec2 mr);
  // Version 1
  void DrawMultiSampleHigherResolutionMode (GLuint ext_screen_output_id);
  void DrawMultiSampleHigherResolutionMode (gl::Texture2D* ext_screen_output = nullptr);
  // Version 1
  void DrawHigherResolutionWithDownScale (GLuint ext_screen_output_id, int screen_output_width, int screen_output_height);
  void DrawHigherResolutionWithDownScale (gl::Texture2D* ext_screen_output = nullptr);
  void DrawLowerResolutionWithUpScale (GLuint ext_screen_output_id, int screen_output_width, int screen_output_height);
  void DrawLowerResolutionWithUpScale (gl::Texture2D* ext_screen_output = nullptr);

  unsigned int GetImageKernelFilter ()
  {
    return m_kernel_filter;
  }
  void SetImageKernelFilter (unsigned int k_i);

  gl::Texture2D* GetScreenOutputTexture ()
  {
    return m_screen_output;
  }
protected:

private:
  void CreateVertexBuffers ();

  gl::Texture2D* m_screen_output;
  gl::Texture2D* m_filtered_screen_output;
  glm::ivec2 m_multisample_res_multiplier;

  std::string m_shader_folder;
  gl::PipelineShader* m_ps_shader;

  gl::ComputeShader* m_cp_shader_multisample;
  gl::ComputeShader* m_cp_shader_downscale;
  gl::ComputeShader* m_cp_shader_upscale;
  gl::ComputeShader* m_cp_shader_digital_filter;
  unsigned int m_kernel_filter;

  gl::ArrayObject* m_cb_vao;
  gl::BufferObject* m_cb_vbo;
  gl::BufferObject* m_cb_ibo;
};

}

#endif