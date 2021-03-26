#include "preillumination.h"

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"

PreIlluminationStructuredVolume::PreIlluminationStructuredVolume (int n_channels)
  : m_active(false)
  , m_light_cache_resolution(glm::ivec3(8))
{
  m_tex_glsl_light_vol_cache = nullptr;

  if (n_channels == 2)
  {
    m_tex_internal_format = GL_RG16F;
    m_tex_data_format = GL_RG;
    m_tex_data_type = GL_FLOAT;
  }
  else //if (n_channels == 1)
  {
    m_tex_internal_format = GL_R16F;
    m_tex_data_format = GL_RED;
    m_tex_data_type = GL_FLOAT;
  }
}

PreIlluminationStructuredVolume::~PreIlluminationStructuredVolume ()
{
  DestroyLightCacheTexture();
}

bool PreIlluminationStructuredVolume::IsActive ()
{
  return m_active;
}

void PreIlluminationStructuredVolume::SetActive (bool f)
{
  m_active = f;
}

glm::ivec3 PreIlluminationStructuredVolume::GetLightCacheResolution ()
{
  return m_light_cache_resolution;
}

void PreIlluminationStructuredVolume::SetLightCacheResolution (glm::ivec3 tex_resolution)
{
  SetLightCacheResolution(tex_resolution.x, tex_resolution.y, tex_resolution.z);
}

void PreIlluminationStructuredVolume::SetLightCacheResolution (int w, int h, int d)
{
  m_light_cache_resolution = glm::ivec3(w, h, d);
}

gl::Texture3D* PreIlluminationStructuredVolume::GetLightCacheTexturePointer ()
{
  return m_tex_glsl_light_vol_cache;
}

void PreIlluminationStructuredVolume::GenerateLightCacheTexture ()
{
  DestroyLightCacheTexture();
  
  if(IsActive())
  {
    // Initialize extinction coefficient volume texture 3D
    m_tex_glsl_light_vol_cache = new gl::Texture3D(m_light_cache_resolution.x,
                                                   m_light_cache_resolution.y,
                                                   m_light_cache_resolution.z);

    // Set initial texture parameters
    m_tex_glsl_light_vol_cache->GenerateTexture(GL_LINEAR, GL_LINEAR,
      GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    
    // Set default data:
    m_tex_glsl_light_vol_cache->SetData(NULL, m_tex_internal_format, m_tex_data_format, m_tex_data_type);
  }
}

void PreIlluminationStructuredVolume::DestroyLightCacheTexture ()
{
  if (m_tex_glsl_light_vol_cache != nullptr) delete m_tex_glsl_light_vol_cache;
  m_tex_glsl_light_vol_cache = nullptr;
}

glm::bvec2 PreIlluminationStructuredVolume::SetImGuiComponents ()
{
  glm::bvec2 ret_l(false, false);

  ImGui::PushID("PreIlluminationLightVolumeCache");
  ImGui::Separator();
  if (ImGui::Checkbox("Use Pre-Illumination###PreIlluminationStructuredVolumePreIlluObjSpace", &m_active))
  {
    GenerateLightCacheTexture();
    ret_l.x = true;
  }
  
  if (IsActive())
  {
    ImGui::BulletText("Resolution:");
    ImGui::PushItemWidth(100.0f);
    ImGui::BeginGroup();
    ImGui::InputInt("###PreIlluminationStructuredVolumeLCWSIZEX", &m_light_cache_resolution.x);
    ImGui::SameLine();
    ImGui::InputInt("###PreIlluminationStructuredVolumeLCHSIZEY", &m_light_cache_resolution.y);
    ImGui::SameLine();
    ImGui::InputInt("###PreIlluminationStructuredVolumeLCDSIZEZ", &m_light_cache_resolution.z);
    if (ImGui::Button("Update"))
    {
      GenerateLightCacheTexture();
      ret_l.y = true;
    }
    ImGui::EndGroup();
    ImGui::PopItemWidth();
  }
  ImGui::Separator();
  ImGui::PopID();

  return ret_l;
}