#include "renderoutputframe.h"

#include <gl_utils/texture2d.h>
#include <gl_utils/pipelineshader.h>
#include <vis_utils/defines.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

#include "defines.h"

#include <vis_utils/filters/kernelbase.hpp>
#include <vis_utils/filters/box_rgba.hpp>
#include <vis_utils/filters/hat_rgba.hpp>
#include <vis_utils/filters/catmullrom_rgba.hpp>
#include <vis_utils/filters/mitchellnetravali_rgba.hpp>
#include <vis_utils/filters/bspline3_rgba.hpp>
#include <vis_utils/filters/omoms3_rgba.hpp>

namespace vis
{

RenderFrameToScreen::RenderFrameToScreen(std::string shader_folder)
  : m_screen_output(nullptr)
  , m_filtered_screen_output(nullptr)
  , m_multisample_res_multiplier(0, 0)
  , m_shader_folder(shader_folder)
  , m_ps_shader(nullptr)
  , m_cp_shader_multisample(nullptr)
  , m_cp_shader_downscale(nullptr)
  , m_cp_shader_upscale(nullptr)
  , m_cp_shader_digital_filter(nullptr)
  , m_kernel_filter(vis::IMAGE_FILTER_KERNEL::K2_HAT)
  , m_cb_vao(nullptr)
  , m_cb_vbo(nullptr)
  , m_cb_ibo(nullptr)
{
}

RenderFrameToScreen::~RenderFrameToScreen ()
{
  Clean();
}

void RenderFrameToScreen::Clean ()
{
  ClearTextures();

  m_multisample_res_multiplier = glm::ivec2(0);

  ClearShaders();

  if (m_cb_vao) delete m_cb_vao;
  m_cb_vao = nullptr;

  if (m_cb_vbo) delete m_cb_vbo;
  m_cb_vbo = nullptr;

  if (m_cb_ibo) delete m_cb_ibo;
  m_cb_ibo = nullptr;
}

void RenderFrameToScreen::UpdateScreenResolution (int s_w, int s_h)
{
  if (m_screen_output == nullptr)
  {
    m_screen_output = new gl::Texture2D(s_w, s_h);
    m_screen_output->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    m_screen_output->SetData(NULL, GL_RGBA16F, GL_RGBA, GL_FLOAT);
  }
  else
  {
    if (m_screen_output->GetWidth() != s_w || m_screen_output->GetHeight() != s_h)
    {
      delete m_screen_output;

      m_screen_output = new gl::Texture2D(s_w, s_h);
      m_screen_output->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      m_screen_output->SetData(NULL, GL_RGBA16F, GL_RGBA, GL_FLOAT);

      if (m_filtered_screen_output) delete m_filtered_screen_output;
      m_filtered_screen_output = nullptr;
    }
  }
  gl::ExitOnGLError("RenderFrameToScreen: Error on UpdateScreenResolution.");
}

void RenderFrameToScreen::UpdateScreenResolutionMultiScaling (int s_w, int s_h, int mw, int mh)
{
  if (mw != 0 && mh != 0)
  {
    if (m_multisample_res_multiplier.x != mw || m_multisample_res_multiplier.y != mh)
      m_multisample_res_multiplier = glm::ivec2(mw, mh);
  }
  else
  {
    mw = m_multisample_res_multiplier.x;
    mh = m_multisample_res_multiplier.y;
  }

  int i_w = s_w;
  if (mw < 0)
    i_w = i_w / std::fabs(mw);
  else
    i_w = i_w * mw;

  int i_h = s_h;
  if (mh < 0)
    i_h = i_h / std::fabs(mh);
  else
    i_h = i_h * mh;

  if (m_screen_output == nullptr)
  {
    m_screen_output = new gl::Texture2D(i_w, i_h);
    m_screen_output->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    m_screen_output->SetData(NULL, GL_RGBA16F, GL_RGBA, GL_FLOAT);

    if (m_filtered_screen_output == nullptr)
    {
      m_filtered_screen_output = new gl::Texture2D(s_w, s_h);
      m_filtered_screen_output->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      m_filtered_screen_output->SetData(NULL, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    }
  }
  else
  {
    if (m_screen_output->GetWidth() != i_w || m_screen_output->GetHeight() != i_h || m_filtered_screen_output == nullptr)
    {
      delete m_screen_output;

      if (m_filtered_screen_output) delete m_filtered_screen_output;
      m_filtered_screen_output = nullptr;

      m_screen_output = new gl::Texture2D(i_w, i_h);
      m_screen_output->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      m_screen_output->SetData(NULL, GL_RGBA16F, GL_RGBA, GL_FLOAT);

      m_filtered_screen_output = new gl::Texture2D(s_w, s_h);
      m_filtered_screen_output->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      m_filtered_screen_output->SetData(NULL, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    }
  }
  gl::ExitOnGLError("RenderFrameToScreen: Error on UpdateScreenResolution.");
}

int RenderFrameToScreen::GetWidth ()
{
  if (m_screen_output) return m_screen_output->GetWidth();
  return 0;
}

int RenderFrameToScreen::GetHeight ()
{
  if (m_screen_output) return m_screen_output->GetHeight();
  return 0;
}

void RenderFrameToScreen::ClearShaders ()
{
  if (m_ps_shader) delete m_ps_shader;
  m_ps_shader = nullptr;

  if (m_cp_shader_multisample) delete m_cp_shader_multisample;
  m_cp_shader_multisample = nullptr;

  if (m_cp_shader_downscale) delete m_cp_shader_downscale;
  m_cp_shader_downscale = nullptr;

  if (m_cp_shader_upscale) delete m_cp_shader_upscale;
  m_cp_shader_upscale = nullptr;

  if (m_cp_shader_digital_filter) delete m_cp_shader_digital_filter;
  m_cp_shader_digital_filter = nullptr;
}

void RenderFrameToScreen::ClearTextures ()
{
  if (m_screen_output) delete m_screen_output;
  m_screen_output = nullptr;

  if (m_filtered_screen_output) delete m_filtered_screen_output;
  m_filtered_screen_output = nullptr;
}

void RenderFrameToScreen::ClearTexture ()
{
  glClearTexImage(m_screen_output->GetTextureID(), 0, GL_RGBA, GL_FLOAT, 0);
}

void RenderFrameToScreen::ClearTextureImage ()
{
  glClearTexImage(m_screen_output->GetTextureID(), 0, GL_RGBA, GL_FLOAT, 0);
}

void RenderFrameToScreen::BindImageTexture (bool multisample)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_screen_output->GetTextureID());
  glBindImageTexture(0, m_screen_output->GetTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
}

void RenderFrameToScreen::Draw ()
{
  Draw(m_screen_output->GetTextureID());
}

void RenderFrameToScreen::Draw (gl::Texture2D* screen_output)
{
  Draw(screen_output->GetTextureID());
}

void RenderFrameToScreen::Draw (GLuint screen_output_id)
{
  if (m_ps_shader == nullptr)
  {
    // Shader to blend the rendered frame to the output screen (used by compute shaders)
    m_ps_shader = new gl::PipelineShader();

    m_ps_shader->AddShaderFile(gl::PipelineShader::TYPE::VERTEX, vis::Utils::GetShaderPath() + "blendframe_render.vert");
    m_ps_shader->AddShaderFile(gl::PipelineShader::TYPE::FRAGMENT, vis::Utils::GetShaderPath() + "blendframe_render.frag");
    m_ps_shader->LoadAndLink();
    m_ps_shader->Bind();
    gl::ExitOnGLError("vis::RenderFrameToScreen: Could not create pipeline blend shader...");

    glm::mat4 projMat = glm::ortho<float>(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
    m_ps_shader->SetUniform("ProjectionMatrix", projMat);
    m_ps_shader->BindUniform("ProjectionMatrix");
    gl::ExitOnGLError("vis::RenderFrameToScreen: Could not bind uniforms...");

    m_ps_shader->Unbind();
    gl::ExitOnGLError("vis::RenderFrameToScreen: Could not unbind pipeline shader...");

    CreateVertexBuffers();
  }

  m_ps_shader->Bind();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, screen_output_id);

  m_ps_shader->SetUniformTexture2D("TexGeneratedFrame", screen_output_id, 0);
  m_ps_shader->BindUniform("TexGeneratedFrame");

  m_cb_vao->Bind();
  m_cb_vbo->Bind();
  m_cb_ibo->Bind();
  m_cb_vao->DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT);
  m_cb_ibo->Unbind();
  m_cb_vbo->Unbind();
  gl::ArrayObject::Unbind();

  gl::ExitOnGLError("vis::RenderFrameToScreen: Could not get shader uniform locations");
  gl::PipelineShader::Unbind();

  glBindTexture(GL_TEXTURE_2D, 0);
}

void RenderFrameToScreen::SetMultiResolutionScreenMultiplier (glm::ivec2 mr)
{
  m_multisample_res_multiplier = mr;
}

void RenderFrameToScreen::DrawMultiSampleHigherResolutionMode(GLuint ext_screen_output_id)
{
  if (m_filtered_screen_output)
  {
    if (m_cp_shader_multisample == nullptr)
    {
      // Shader to blend the rendered frame to the output screen (used by compute shaders)
      m_cp_shader_multisample = new gl::ComputeShader();

      m_cp_shader_multisample->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/multisample_filter.comp");
      m_cp_shader_multisample->LoadAndLink();
      m_cp_shader_multisample->Bind();
      gl::ExitOnGLError("vis::RenderFrameToScreen: Could not create compute shader for multisample...");

      m_cp_shader_multisample->Unbind();
      gl::ExitOnGLError("vis::RenderFrameToScreen: Could not unbind compute shader...");
    }

    m_cp_shader_multisample->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_filtered_screen_output->GetTextureID());
    glBindImageTexture(0, m_filtered_screen_output->GetTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

    m_cp_shader_multisample->RecomputeNumberOfGroups(m_filtered_screen_output->GetWidth(),
      m_filtered_screen_output->GetHeight(), 1);

    m_cp_shader_multisample->SetUniformTexture2D("TexGeneratedFrame", ext_screen_output_id, 1);
    m_cp_shader_multisample->BindUniform("TexGeneratedFrame");

    m_cp_shader_multisample->Dispatch();

    Draw(m_filtered_screen_output->GetTextureID());
  }
}

void RenderFrameToScreen::DrawMultiSampleHigherResolutionMode (gl::Texture2D* ext_screen_output)
{
  gl::Texture2D* screen_output = ext_screen_output ? ext_screen_output : m_screen_output;
  DrawMultiSampleHigherResolutionMode(screen_output->GetTextureID());
}

void RenderFrameToScreen::DrawHigherResolutionWithDownScale (GLuint ext_screen_output_id, int screen_output_width, int screen_output_height)
{
  if (m_filtered_screen_output)
  {
    if (m_cp_shader_downscale == nullptr)
    {
      // Shader to blend the rendered frame to the output screen (used by compute shaders)
      m_cp_shader_downscale = new gl::ComputeShader();
      if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K1_BOX)
      {
        m_cp_shader_downscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/box_filter.comp");
      }
      else if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K2_HAT)
      {
        m_cp_shader_downscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/hat_filter.comp");
      }
      else if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CATMULL_ROM)
      {
        m_cp_shader_downscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/catmullrom_filter.comp");
      }
      else if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_MITCHELL_NETRAVALI)
      {
        m_cp_shader_downscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/mitchellnetravali_filter.comp");
      }
      else if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CARDINAL_BSPLINE_3)
      {
        m_cp_shader_downscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/cardinalbspline_filter.comp");
      }
      else if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CARDINAL_OMOMS3)
      {
        m_cp_shader_downscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/cardinalomoms_filter.comp");
      }
      m_cp_shader_downscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/downscaling_filter.comp");
      m_cp_shader_downscale->LoadAndLink();
      m_cp_shader_downscale->Bind();
      gl::ExitOnGLError("vis::RenderFrameToScreen: Could not create compute shader for multisample...");

      m_cp_shader_downscale->Unbind();
      gl::ExitOnGLError("vis::RenderFrameToScreen: Could not unbind compute shader...");
    }

    m_cp_shader_downscale->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_filtered_screen_output->GetTextureID());
    glBindImageTexture(0, m_filtered_screen_output->GetTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

    m_cp_shader_downscale->RecomputeNumberOfGroups(m_filtered_screen_output->GetWidth(),
      m_filtered_screen_output->GetHeight(), 0);

    m_cp_shader_downscale->SetUniform("TexGeneratedWidth", (int)screen_output_width);
    m_cp_shader_downscale->BindUniform("TexGeneratedWidth");
    m_cp_shader_downscale->SetUniform("TexGeneratedHeight", (int)screen_output_height);
    m_cp_shader_downscale->BindUniform("TexGeneratedHeight");
    m_cp_shader_downscale->SetUniformTexture2D("TexGeneratedFrame", ext_screen_output_id, 1);
    m_cp_shader_downscale->BindUniform("TexGeneratedFrame");

    m_cp_shader_downscale->SetUniform("TargetWidth", (int)m_filtered_screen_output->GetWidth());
    m_cp_shader_downscale->BindUniform("TargetWidth");
    m_cp_shader_downscale->SetUniform("TargetHeight", (int)m_filtered_screen_output->GetHeight());
    m_cp_shader_downscale->BindUniform("TargetHeight");

    m_cp_shader_downscale->Dispatch();
    m_cp_shader_downscale->Unbind();

    if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CARDINAL_BSPLINE_3 || m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CARDINAL_OMOMS3)
    {
      if (m_cp_shader_digital_filter == nullptr)
      {
        m_cp_shader_digital_filter = new gl::ComputeShader();
        if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CARDINAL_BSPLINE_3)
          m_cp_shader_digital_filter->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/cbs_digital_filter.comp");
        else if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CARDINAL_OMOMS3)
          m_cp_shader_digital_filter->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/comoms_digital_filter.comp");
        m_cp_shader_digital_filter->LoadAndLink();
        m_cp_shader_digital_filter->Bind();
        m_cp_shader_digital_filter->Unbind();
      }

      m_cp_shader_digital_filter->Bind();

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, m_filtered_screen_output->GetTextureID());
      glBindImageTexture(0, m_filtered_screen_output->GetTextureID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);


      m_cp_shader_digital_filter->SetUniform("TexWidth", (int)m_filtered_screen_output->GetWidth());
      m_cp_shader_digital_filter->BindUniform("TexWidth");
      m_cp_shader_digital_filter->SetUniform("TexHeight", (int)m_filtered_screen_output->GetHeight());
      m_cp_shader_digital_filter->BindUniform("TexHeight");

      m_cp_shader_digital_filter->SetUniform("FilterDirection", (int)0);
      m_cp_shader_digital_filter->BindUniform("FilterDirection");

      m_cp_shader_digital_filter->RecomputeNumberOfGroups(m_filtered_screen_output->GetHeight(), 0, 0);
      m_cp_shader_digital_filter->Dispatch();

      m_cp_shader_digital_filter->SetUniform("FilterDirection", (int)1);
      m_cp_shader_digital_filter->BindUniform("FilterDirection");

      m_cp_shader_digital_filter->RecomputeNumberOfGroups(m_filtered_screen_output->GetWidth(), 0, 0);
      m_cp_shader_digital_filter->Dispatch();

      m_cp_shader_digital_filter->Unbind();
    }

    Draw(m_filtered_screen_output->GetTextureID());
  }
}

void RenderFrameToScreen::DrawHigherResolutionWithDownScale (gl::Texture2D* ext_screen_output)
{
  gl::Texture2D* screen_output = ext_screen_output ? ext_screen_output : m_screen_output;
  DrawHigherResolutionWithDownScale(screen_output->GetTextureID(), screen_output->GetWidth(), screen_output->GetHeight());
}

void RenderFrameToScreen::DrawLowerResolutionWithUpScale (GLuint ext_screen_output_id, int screen_output_width, int screen_output_height)
{
  if (m_filtered_screen_output)
  {
    if (m_cp_shader_upscale == nullptr)
    {
      // Shader to blend the rendered frame to the output screen (used by compute shaders)
      m_cp_shader_upscale = new gl::ComputeShader();

      if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K1_BOX)
      {
        m_cp_shader_upscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/box_filter.comp");
      }
      else if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K2_HAT)
      {
        m_cp_shader_upscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/hat_filter.comp");
      }
      else if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CATMULL_ROM)
      {
        m_cp_shader_upscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/catmullrom_filter.comp");
      }
      else if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_MITCHELL_NETRAVALI)
      {
        m_cp_shader_upscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/mitchellnetravali_filter.comp");
      }
      else if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CARDINAL_BSPLINE_3)
      {
        m_cp_shader_upscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/cardinalbspline_filter.comp");
      }
      else if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CARDINAL_OMOMS3)
      {
        m_cp_shader_upscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/cardinalomoms_filter.comp");
      }
      m_cp_shader_upscale->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/upscaling_filter.comp");
      m_cp_shader_upscale->LoadAndLink();
      m_cp_shader_upscale->Bind();
      gl::ExitOnGLError("vis::RenderFrameToScreen: Could not create compute shader for upsampling...");

      m_cp_shader_upscale->Unbind();
      gl::ExitOnGLError("vis::RenderFrameToScreen: Could not unbind upsampling compute shader...");
    }

    if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CARDINAL_BSPLINE_3 || m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CARDINAL_OMOMS3)
    {
      if (m_cp_shader_digital_filter == nullptr)
      {
        m_cp_shader_digital_filter = new gl::ComputeShader();
        if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CARDINAL_BSPLINE_3)
          m_cp_shader_digital_filter->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/cbs_digital_filter.comp");
        else if (m_kernel_filter == vis::IMAGE_FILTER_KERNEL::K4_CARDINAL_OMOMS3)
          m_cp_shader_digital_filter->AddShaderFile(vis::Utils::GetShaderPath() + "renderoutputframe/comoms_digital_filter.comp");
        m_cp_shader_digital_filter->LoadAndLink();
        m_cp_shader_digital_filter->Bind();
        m_cp_shader_digital_filter->Unbind();
      }

      m_cp_shader_digital_filter->Bind();

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, ext_screen_output_id);
      glBindImageTexture(0, ext_screen_output_id, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

      m_cp_shader_digital_filter->RecomputeNumberOfGroups(screen_output_width, 0, 0);

      m_cp_shader_digital_filter->SetUniform("TexWidth", (int)screen_output_width);
      m_cp_shader_digital_filter->BindUniform("TexWidth");
      m_cp_shader_digital_filter->SetUniform("TexHeight", (int)screen_output_height);
      m_cp_shader_digital_filter->BindUniform("TexHeight");

      m_cp_shader_digital_filter->SetUniform("FilterDirection", (int)0);
      m_cp_shader_digital_filter->BindUniform("FilterDirection");

      m_cp_shader_digital_filter->RecomputeNumberOfGroups(m_filtered_screen_output->GetHeight(), 0, 0);
      m_cp_shader_digital_filter->Dispatch();

      m_cp_shader_digital_filter->SetUniform("FilterDirection", (int)1);
      m_cp_shader_digital_filter->BindUniform("FilterDirection");

      m_cp_shader_digital_filter->RecomputeNumberOfGroups(m_filtered_screen_output->GetWidth(), 0, 0);
      m_cp_shader_digital_filter->Dispatch();

      m_cp_shader_digital_filter->Unbind();
    }

    m_cp_shader_upscale->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_filtered_screen_output->GetTextureID());
    glBindImageTexture(0, m_filtered_screen_output->GetTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

    m_cp_shader_upscale->RecomputeNumberOfGroups(m_filtered_screen_output->GetWidth(),
      m_filtered_screen_output->GetHeight(), 0);

    m_cp_shader_upscale->SetUniform("TexGeneratedWidth", (int)screen_output_width);
    m_cp_shader_upscale->BindUniform("TexGeneratedWidth");
    m_cp_shader_upscale->SetUniform("TexGeneratedHeight", (int)screen_output_height);
    m_cp_shader_upscale->BindUniform("TexGeneratedHeight");

    m_cp_shader_upscale->SetUniform("TargetWidth", (int)m_filtered_screen_output->GetWidth());
    m_cp_shader_upscale->BindUniform("TargetWidth");
    m_cp_shader_upscale->SetUniform("TargetHeight", (int)m_filtered_screen_output->GetHeight());
    m_cp_shader_upscale->BindUniform("TargetHeight");

    m_cp_shader_upscale->SetUniformTexture2D("TexGeneratedFrame", ext_screen_output_id, 1);
    m_cp_shader_upscale->BindUniform("TexGeneratedFrame");

    m_cp_shader_upscale->Dispatch();
    m_cp_shader_upscale->Unbind();
    Draw(m_filtered_screen_output->GetTextureID());
  }
}

void RenderFrameToScreen::DrawLowerResolutionWithUpScale (gl::Texture2D* ext_screen_output)
{
  gl::Texture2D* screen_output = ext_screen_output ? ext_screen_output : m_screen_output;
  DrawLowerResolutionWithUpScale(screen_output->GetTextureID(), screen_output->GetWidth(), screen_output->GetHeight());
}

void RenderFrameToScreen::SetImageKernelFilter (unsigned int k_i)
{
  m_kernel_filter = k_i;
  ClearShaders();
}

void RenderFrameToScreen::CreateVertexBuffers ()
{
  const GLfloat VERTICES[12] = { -1.0f, -1.0f, 0.0f,
                                 +1.0f, -1.0f, 0.0f,
                                 +1.0f, +1.0f, 0.0f,
                                 -1.0f, +1.0f, 0.0f };
  const GLuint INDICES[6] = { 0, 1, 2, 0, 2, 3 };

  if (m_cb_vao == nullptr)
  {
    m_cb_vao = new gl::ArrayObject(1);
    m_cb_vao->Bind();

    m_cb_vbo = new gl::BufferObject(gl::BufferObject::TYPES::VERTEXBUFFEROBJECT);
    m_cb_ibo = new gl::BufferObject(gl::BufferObject::TYPES::INDEXBUFFEROBJECT);

    //bind the VBO to the VAO
    m_cb_vbo->SetBufferData(sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);

    m_cb_vao->SetVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3.0f, (GLvoid*)0);

    //bind the IBO to the VAO
    m_cb_ibo->SetBufferData(sizeof(INDICES), INDICES, GL_STATIC_DRAW);

    m_cb_ibo->Unbind();
    m_cb_vbo->Unbind();
    gl::ArrayObject::Unbind();
  }
}

}