#include "../../defines.h"
#include "vctrenderer.h"

#include <math_utils/utils.h>

#include <gl_utils/pipelineshader.h>
#include <gl_utils/camera.h>

#include "../../renderingmanager.h"
#include <glm/gtc/type_ptr.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <vis_utils/camera.h>

#include <volvis_utils/utils.h>

#include <gl_utils/texture2d.h>
#include <gl_utils/texture3d.h>

/////////////////////////////////
// public functions
/////////////////////////////////
RC1PVoxelConeTracingSGPU::RC1PVoxelConeTracingSGPU ()
  : m_glsl_transfer_function(nullptr)
  , cp_shader_rendering(nullptr)
  , m_u_step_size(0.5f)
  , m_apply_gradient_shading(false)
{
  apply_ambient_occlusion      = true;

  apply_voxel_cone_tracing     = true;
  cone_step_size               = 2.0f;
  cone_step_size_increase_rate = 1.0f;
  cone_initial_step            = 2.0f;
  cone_apex_angle              = 2.0f; // Shadow softness
  apply_correction_factor      = true;
  opacity_correction_factor    = 2.0f; // Shadow darkness
  cone_number_of_samples       = 50;
  covered_distance             = 0.0f;

  //////////////////////////////////////////
  // Light Cache
  m_pre_illum_str_vol.SetActive(false);
  m_pre_illum_str_vol.SetLightCacheResolution(32, 32, 32);
  cp_lightcache_shader = nullptr;

#ifdef MULTISAMPLE_AVAILABLE
  vr_pixel_multiscaling_support = true;
#endif
}

RC1PVoxelConeTracingSGPU::~RC1PVoxelConeTracingSGPU ()
{
  Clean();
}

void RC1PVoxelConeTracingSGPU::Clean ()
{
  if (m_glsl_transfer_function) delete m_glsl_transfer_function;
  m_glsl_transfer_function = nullptr;

  m_pre_illum_str_vol.DestroyLightCacheTexture();
  
  if (cp_lightcache_shader != nullptr)
    delete cp_lightcache_shader;
  cp_lightcache_shader = nullptr;

  pre_processing.Destroy();

  DestroyRenderingShaders();

  BaseVolumeRenderer::Clean();
}

void RC1PVoxelConeTracingSGPU::ReloadShaders ()
{
  cp_shader_rendering->Reload();
  if (cp_lightcache_shader) cp_lightcache_shader->Reload();
  m_rdr_frame_to_screen.ClearShaders();
}

bool RC1PVoxelConeTracingSGPU::Init (int swidth, int sheight)
{
  if (IsBuilt()) Clean();

  if (m_ext_data_manager->GetCurrentVolumeTexture() == nullptr) return false;
  m_glsl_transfer_function = m_ext_data_manager->GetCurrentTransferFunction()->GenerateTexture_1D_RGBt();

  // Pre Processing stage to compute supervoxels and preintegration table
  pre_processing.PreProcessSuperVoxels(m_ext_data_manager->GetCurrentStructuredVolume());
  pre_processing.PreProcessPreIntegrationTable(m_ext_data_manager->GetCurrentStructuredVolume(), m_ext_data_manager->GetCurrentTransferFunction());

  m_pre_illum_str_vol.GenerateLightCacheTexture();

  SetOutdated();
  CreateRenderingPass();

  // Get the current Diagonal of the Volume
  vis::StructuredGridVolume* vold = m_ext_data_manager->GetCurrentStructuredVolume();
  float v_w = (float)vold->GetWidth()  * vold->GetScaleX();
  float v_h = (float)vold->GetHeight() * vold->GetScaleY();
  float v_d = (float)vold->GetDepth()  * vold->GetScaleZ();
  float Dv = glm::sqrt(v_w*v_w + v_h * v_h + v_d * v_d);

  gl::ExitOnGLError("Error on Preparing Models and Shaders");
  gl::ArrayObject::Unbind();
  gl::PipelineShader::Unbind();

  // estimate initial integration step
  glm::dvec3 sv = m_ext_data_manager->GetCurrentStructuredVolume()->GetScale();
  m_u_step_size = float((0.5f / glm::sqrt(3.0f)) * glm::sqrt(sv.x * sv.x + sv.y * sv.y + sv.z * sv.z));

  Reshape(swidth, sheight);

  SetBuilt(true);
  SetOutdated();
  return true;
}

bool RC1PVoxelConeTracingSGPU::Update (vis::Camera* camera)
{
  if (m_pre_illum_str_vol.IsActive())
  {
    PreComputeLightCache(camera);

    cp_shader_rendering->Bind();

    cp_shader_rendering->SetUniformTexture3D("TexVolumeLightCache", m_pre_illum_str_vol.GetLightCacheTexturePointer()->GetTextureID(), 4);
    cp_shader_rendering->BindUniform("TexVolumeLightCache");
  }
  else // image space
  {
    cp_shader_rendering->Bind();

    cp_shader_rendering->SetUniformTexture3D("TexSuperVoxelsVolume", pre_processing.glsl_supervoxel_meanstddev->GetTextureID(), 4);
    cp_shader_rendering->BindUniform("TexSuperVoxelsVolume");

    cp_shader_rendering->SetUniformTexture2D("TexPreIntegrationLookup", pre_processing.glsl_preintegration_lookup->GetTextureID(), 5);
    cp_shader_rendering->BindUniform("TexPreIntegrationLookup");

    cp_shader_rendering->SetUniform("TanRadiusConeApexAngle", glm::tan(cone_apex_angle * glm::pi<float>() / 180.0f));
    cp_shader_rendering->BindUniform("TanRadiusConeApexAngle");

    cp_shader_rendering->SetUniform("ConeStepSize", cone_step_size);
    cp_shader_rendering->BindUniform("ConeStepSize");

    cp_shader_rendering->SetUniform("ConeStepIncreaseRate", cone_step_size_increase_rate);
    cp_shader_rendering->BindUniform("ConeStepIncreaseRate");

    cp_shader_rendering->SetUniform("ConeInitialStep", cone_initial_step);
    cp_shader_rendering->BindUniform("ConeInitialStep");

    cp_shader_rendering->SetUniform("OpacityCorrectionFactor", opacity_correction_factor);
    cp_shader_rendering->BindUniform("OpacityCorrectionFactor");

    cp_shader_rendering->SetUniform("ApplyOpacityCorrectionFactor", apply_correction_factor ? 1 : 0);
    cp_shader_rendering->BindUniform("ApplyOpacityCorrectionFactor");

    cp_shader_rendering->SetUniform("ConeNumberOfSamples", cone_number_of_samples);
    cp_shader_rendering->BindUniform("ConeNumberOfSamples");

    cp_shader_rendering->SetUniform("VolumeMaxDensity", (float)m_ext_data_manager->GetCurrentStructuredVolume()->GetMaxDensity());
    cp_shader_rendering->BindUniform("VolumeMaxDensity");

    cp_shader_rendering->SetUniform("VolumeMaxStandardDeviation", (float)pre_processing.maximum_standard_deviation);
    cp_shader_rendering->BindUniform("VolumeMaxStandardDeviation");
  }

  cp_shader_rendering->Bind();

  // MULTISAMPLE
  if (IsPixelMultiScalingSupported() && GetCurrentMultiScalingMode() > 0)
  {
    cp_shader_rendering->RecomputeNumberOfGroups(m_rdr_frame_to_screen.GetWidth(),
      m_rdr_frame_to_screen.GetHeight(), 0);
  }
  else
  {
    cp_shader_rendering->RecomputeNumberOfGroups(m_ext_rendering_parameters->GetScreenWidth(),
      m_ext_rendering_parameters->GetScreenHeight(), 0);
  }

  cp_shader_rendering->SetUniform("CameraEye", camera->GetEye());
  cp_shader_rendering->BindUniform("CameraEye");

  cp_shader_rendering->SetUniform("ViewMatrix", camera->LookAt());
  cp_shader_rendering->BindUniform("ViewMatrix");

  cp_shader_rendering->SetUniform("ProjectionMatrix", camera->Projection());
  cp_shader_rendering->BindUniform("ProjectionMatrix");

  cp_shader_rendering->SetUniform("fov_y_tangent", (float)tan((camera->GetFovY() / 2.0) * glm::pi<double>() / 180.0));
  cp_shader_rendering->BindUniform("fov_y_tangent");

  cp_shader_rendering->SetUniform("aspect_ratio", camera->GetAspectRatio());
  cp_shader_rendering->BindUniform("aspect_ratio");

  cp_shader_rendering->SetUniform("ApplyOcclusion", apply_ambient_occlusion ? 1 : 0);
  cp_shader_rendering->BindUniform("ApplyOcclusion");

  cp_shader_rendering->SetUniform("ApplyShadow", apply_voxel_cone_tracing ? 1 : 0);
  cp_shader_rendering->BindUniform("ApplyShadow");

  cp_shader_rendering->SetUniform("StepSize", m_u_step_size);
  cp_shader_rendering->BindUniform("StepSize");

  cp_shader_rendering->SetUniform("ApplyPhongShading", (m_apply_gradient_shading && m_ext_data_manager->GetCurrentGradientTexture()) ? 1 : 0);
  cp_shader_rendering->BindUniform("ApplyPhongShading");

  cp_shader_rendering->SetUniform("Kambient", m_ext_rendering_parameters->GetBlinnPhongKambient());
  cp_shader_rendering->BindUniform("Kambient");
  cp_shader_rendering->SetUniform("Kdiffuse", m_ext_rendering_parameters->GetBlinnPhongKdiffuse());
  cp_shader_rendering->BindUniform("Kdiffuse");
  cp_shader_rendering->SetUniform("Kspecular", m_ext_rendering_parameters->GetBlinnPhongKspecular());
  cp_shader_rendering->BindUniform("Kspecular");
  cp_shader_rendering->SetUniform("Nshininess", m_ext_rendering_parameters->GetBlinnPhongNshininess());
  cp_shader_rendering->BindUniform("Nshininess");

  cp_shader_rendering->SetUniform("Ispecular", m_ext_rendering_parameters->GetLightSourceSpecular());
  cp_shader_rendering->BindUniform("Ispecular");

  cp_shader_rendering->SetUniform("WorldEyePos", camera->GetEye());
  cp_shader_rendering->BindUniform("WorldEyePos");

  cp_shader_rendering->SetUniform("WorldLightingPos", m_ext_rendering_parameters->GetBlinnPhongLightingPosition());
  cp_shader_rendering->BindUniform("WorldLightingPos");

  cp_shader_rendering->BindUniforms();

  gl::Shader::Unbind();
  gl::ExitOnGLError("RC1PConeTracingDirOcclusionShading: After Update.");
  return true;
}

void RC1PVoxelConeTracingSGPU::Redraw ()
{
  m_rdr_frame_to_screen.ClearTexture();

  cp_shader_rendering->Bind();
  m_rdr_frame_to_screen.BindImageTexture();

  cp_shader_rendering->Dispatch();
  gl::ComputeShader::Unbind();

  m_rdr_frame_to_screen.Draw();
}

void RC1PVoxelConeTracingSGPU::MultiSampleRedraw ()
{
  m_rdr_frame_to_screen.ClearTexture();

  cp_shader_rendering->Bind();
  m_rdr_frame_to_screen.BindImageTexture();

  cp_shader_rendering->Dispatch();
  gl::ComputeShader::Unbind();

  m_rdr_frame_to_screen.DrawMultiSampleHigherResolutionMode();
}

void RC1PVoxelConeTracingSGPU::DownScalingRedraw ()
{
  m_rdr_frame_to_screen.ClearTexture();

  cp_shader_rendering->Bind();
  m_rdr_frame_to_screen.BindImageTexture();

  cp_shader_rendering->Dispatch();
  gl::ComputeShader::Unbind();

  m_rdr_frame_to_screen.DrawHigherResolutionWithDownScale();
}

void RC1PVoxelConeTracingSGPU::UpScalingRedraw ()
{
  m_rdr_frame_to_screen.ClearTexture();

  cp_shader_rendering->Bind();
  m_rdr_frame_to_screen.BindImageTexture();

  cp_shader_rendering->Dispatch();
  gl::ComputeShader::Unbind();

  m_rdr_frame_to_screen.DrawLowerResolutionWithUpScale();
}

void RC1PVoxelConeTracingSGPU::SetImGuiComponents ()
{
  ImGui::Separator();
  ImGui::Text("- Step Size: ");
  if (ImGui::DragFloat("###RayCasting1PassUIIntegrationStepSize", &m_u_step_size, 0.01f, 0.01f, 100.0f, "%.2f"))
    SetOutdated();

  AddImGuiMultiSampleOptions();

  if (m_ext_data_manager->GetCurrentGradientTexture())
  {
    ImGui::Separator();
    if (ImGui::Checkbox("Apply Gradient Shading", &m_apply_gradient_shading))
    {
      // Delete current uniform
      cp_shader_rendering->ClearUniform("TexVolumeGradient");

      if (m_apply_gradient_shading && m_ext_data_manager->GetCurrentGradientTexture())
      {
        cp_shader_rendering->Bind();
        cp_shader_rendering->SetUniformTexture3D("TexVolumeGradient", m_ext_data_manager->GetCurrentGradientTexture()->GetTextureID(), 3);
        cp_shader_rendering->BindUniform("TexVolumeGradient");
        gl::ComputeShader::Unbind();
      }
      SetOutdated();
    }
    ImGui::Separator();
  }

  // Pre-Illumination
  glm::bvec2 ret_lc = m_pre_illum_str_vol.SetImGuiComponents();
  if (ret_lc.x)
  {
    DestroyRenderingShaders();
    CreateRenderingPass();
  }
  else if (ret_lc.y)
  {
    SetOutdated();
  }

  ImGui::PushID("Cone Occlusion Data");
  ImGui::Text("- Occlusion Parameters");
  if (ImGui::Checkbox("Apply Ambient Occlusion", &apply_ambient_occlusion))
  {
    SetOutdated();
  }
  ImGui::PopID();

  ImGui::PushID("Cone Shadow Data");
  ImGui::Text("- Shadow Parameters");
  if (ImGui::Checkbox("Apply VCT Shadow", &apply_voxel_cone_tracing))
  {
    SetOutdated();
  }
  
  ImGui::Text("Cone Aperture Angle");
  if (ImGui::DragFloat("###ShadowConeAperture", &cone_apex_angle, 0.5f, 0.5f, 89.5f))
  {
    SetOutdated();
  }

  ImGui::Text("Cone Step Size");
  if (ImGui::DragFloat("###ShadowConeStepSize", &cone_step_size, 0.5f, 0.5f, 100.0f))
  {
    SetOutdated();
  }

  ImGui::Text("Cone Step Size Increase Rate");
  if (ImGui::DragFloat("###ShadowConeStepSizeIncreaseRate", &cone_step_size_increase_rate, 0.01f, 1.0f, 100.0f))
  {
    SetOutdated();
  }

  ImGui::Text("Initial GapStep");
  if (ImGui::DragFloat("###ShadowInitialGapStep", &cone_initial_step, 0.01f, 0.01f, 10.0f))
  {
    SetOutdated();
  }

  if (ImGui::Checkbox("Apply  Correction Factor", &apply_correction_factor))
  {
    SetOutdated();
  }

  ImGui::Text("Opacity Correction Factor");
  if (ImGui::DragFloat("###ShadowOpacityCorrectionFactor", &opacity_correction_factor, 0.01f, 0.01f, 10.0f))
  {
    SetOutdated();
  }

  ImGui::Text("Number of Samples");
  if (ImGui::DragInt("###VCTNumberOfSamples", &cone_number_of_samples, 1, 0, 10000))
  {
    SetOutdated();
  }
  ImGui::PopID();
}

/////////////////////////////////
// protected/private functions
/////////////////////////////////
void RC1PVoxelConeTracingSGPU::PreComputeLightCache (vis::Camera* camera)
{
  vis::StructuredGridVolume* vol = m_ext_data_manager->GetCurrentStructuredVolume();

  glm::vec3 vol_scale = glm::vec3(m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleX(),
                                  m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleY(),
                                  m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleZ());

  cp_lightcache_shader->Bind();

  glActiveTexture(GL_TEXTURE0);

  int w = m_ext_data_manager->GetCurrentStructuredVolume()->GetWidth();
  int h = m_ext_data_manager->GetCurrentStructuredVolume()->GetHeight();
  int d = m_ext_data_manager->GetCurrentStructuredVolume()->GetDepth();

  glBindTexture(GL_TEXTURE_3D, m_pre_illum_str_vol.GetLightCacheTexturePointer()->GetTextureID());

  // First, evaluate for level 0.0
  // 3D texture: layered must be true (???)
  // https://stackoverflow.com/questions/17015132/compute-shader-not-modifying-3d-texture
  // https://stackoverflow.com/questions/37136813/what-is-the-difference-between-glbindimagetexture-and-glbindtexture
  // https://www.khronos.org/opengl/wiki/GLAPI/glBindImageTexture
  glBindImageTexture(0, m_pre_illum_str_vol.GetLightCacheTexturePointer()->GetTextureID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG16F);

  cp_lightcache_shader->SetUniform("LightCacheDimensions", glm::vec3(m_pre_illum_str_vol.GetLightCacheTexturePointer()->GetWidth(),
                                                                     m_pre_illum_str_vol.GetLightCacheTexturePointer()->GetHeight(),
                                                                     m_pre_illum_str_vol.GetLightCacheTexturePointer()->GetDepth()));
  cp_lightcache_shader->BindUniform("LightCacheDimensions");

  // Upload volume dimensions
  cp_lightcache_shader->SetUniform("VolumeDimensions", glm::vec3((float)vol->GetWidth(), (float)vol->GetHeight(), (float)vol->GetDepth()));
  cp_lightcache_shader->BindUniform("VolumeDimensions");

  cp_lightcache_shader->SetUniform("VolumeScales", glm::vec3(vol->GetScale()));
  cp_lightcache_shader->BindUniform("VolumeScales");

  // Upload volume scaled sizes
  float sw = vol->GetWidth(); float sh = vol->GetHeight(); float sd = vol->GetDepth();
  cp_lightcache_shader->SetUniform("VolumeScaledSizes", glm::vec3(sw * vol->GetScaleX(), sh * vol->GetScaleY(), sd * vol->GetScaleZ()));
  cp_lightcache_shader->BindUniform("VolumeScaledSizes");

  // Bind volume texture
  cp_lightcache_shader->SetUniformTexture3D("TexVolume", m_ext_data_manager->GetCurrentVolumeTexture()->GetTextureID(), 1);
  cp_lightcache_shader->BindUniform("TexVolume");

  // Bind transfer function texture
  cp_lightcache_shader->SetUniformTexture1D("TexTransferFunc", m_glsl_transfer_function->GetTextureID(), 2);
  cp_lightcache_shader->BindUniform("TexTransferFunc");

  // Bind Tree Super Voxel
  cp_lightcache_shader->SetUniformTexture3D("TexSuperVoxelsVolume", pre_processing.glsl_supervoxel_meanstddev->GetTextureID(), 3);
  cp_lightcache_shader->BindUniform("TexSuperVoxelsVolume");

  // Bind Pre Integration Look Up Table
  cp_lightcache_shader->SetUniformTexture2D("TexPreIntegrationLookup", pre_processing.glsl_preintegration_lookup->GetTextureID(), 4);
  cp_lightcache_shader->BindUniform("TexPreIntegrationLookup");

  // Upload Voxel Cone Tracing parameters
  cp_lightcache_shader->SetUniform("ConeStepSize", cone_step_size);
  cp_lightcache_shader->BindUniform("ConeStepSize");

  cp_lightcache_shader->SetUniform("ConeStepIncreaseRate", cone_step_size_increase_rate);
  cp_lightcache_shader->BindUniform("ConeStepIncreaseRate");

  cp_lightcache_shader->SetUniform("ConeInitialStep", cone_initial_step);
  cp_lightcache_shader->BindUniform("ConeInitialStep");

  cp_lightcache_shader->SetUniform("RadiusConeApexAngle", cone_apex_angle * glm::pi<float>() / 180.0f);
  cp_lightcache_shader->BindUniform("RadiusConeApexAngle");

  // We don't divide by 2 since our angle is the half angle of the cone
  //
  // \    |    /
  //  \   |   /
  //   \  |  /
  //    \ | /
  //     \|/
  //      *
  cp_lightcache_shader->SetUniform("TanRadiusConeApexAngle", glm::tan(cone_apex_angle * glm::pi<float>() / 180.0f));
  cp_lightcache_shader->BindUniform("TanRadiusConeApexAngle");

  cp_lightcache_shader->SetUniform("ApplyOpacityCorrectionFactor", apply_correction_factor ? 1 : 0);
  cp_lightcache_shader->BindUniform("ApplyOpacityCorrectionFactor");

  cp_lightcache_shader->SetUniform("OpacityCorrectionFactor", opacity_correction_factor);
  cp_lightcache_shader->BindUniform("OpacityCorrectionFactor");

  cp_lightcache_shader->SetUniform("ConeNumberOfSamples", cone_number_of_samples);
  cp_lightcache_shader->BindUniform("ConeNumberOfSamples");

  cp_lightcache_shader->SetUniform("VolumeMaxDensity", (float)vol->GetMaxDensity());
  cp_lightcache_shader->BindUniform("VolumeMaxDensity");

  cp_lightcache_shader->SetUniform("VolumeMaxStandardDeviation", (float)pre_processing.maximum_standard_deviation);
  cp_lightcache_shader->BindUniform("VolumeMaxStandardDeviation");

  // Upload light position
  glm::vec3 lpos = m_ext_rendering_parameters->GetBlinnPhongLightingPosition();
  cp_lightcache_shader->SetUniform("WorldLightingPos", lpos);
  cp_lightcache_shader->BindUniform("WorldLightingPos");

  cp_lightcache_shader->SetUniform("ApplyOcclusion", apply_ambient_occlusion ? 1 : 0);
  cp_lightcache_shader->BindUniform("ApplyOcclusion");

  cp_lightcache_shader->SetUniform("ApplyShadow", apply_voxel_cone_tracing ? 1 : 0);
  cp_lightcache_shader->BindUniform("ApplyShadow");

  glActiveTexture(GL_TEXTURE0);
  cp_lightcache_shader->RecomputeNumberOfGroups(
    m_pre_illum_str_vol.GetLightCacheTexturePointer()->GetWidth(),
    m_pre_illum_str_vol.GetLightCacheTexturePointer()->GetHeight(),
    m_pre_illum_str_vol.GetLightCacheTexturePointer()->GetDepth()
  );

  cp_lightcache_shader->Dispatch();

  glBindTexture(GL_TEXTURE_3D, 0);
  glActiveTexture(GL_TEXTURE0);

  gl::Shader::Unbind();
  gl::ExitOnGLError("ERROR: After SetData");
}

void RC1PVoxelConeTracingSGPU::CreateRenderingPass ()
{
  glm::vec3 vol_resolution = glm::vec3(m_ext_data_manager->GetCurrentStructuredVolume()->GetWidth(),
    m_ext_data_manager->GetCurrentStructuredVolume()->GetHeight(),
    m_ext_data_manager->GetCurrentStructuredVolume()->GetDepth());

  glm::vec3 vol_voxelsize = glm::vec3(m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleX(),
    m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleY(),
    m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleZ());

  glm::vec3 vol_aabb = vol_resolution * vol_voxelsize;

  cp_shader_rendering = new gl::ComputeShader();

  if (m_pre_illum_str_vol.IsActive())
  {
    cp_shader_rendering->SetShaderFile(CPPVOLREND_DIR"structured/_common_shaders/obj_ray_marching.comp");
  }
  else
  {
    cp_shader_rendering->SetShaderFile(CPPVOLREND_DIR"structured/rc1pvctsg/vct_ray_bbox_marching.comp");
  }
  
  cp_shader_rendering->LoadAndLink();

  cp_shader_rendering->Bind();

  cp_shader_rendering->SetUniform("VolumeScaledSizes", vol_aabb);
  cp_shader_rendering->SetUniform("VolumeScales", vol_voxelsize);


  // Bind volume rendering textures
  if (m_ext_data_manager->GetCurrentVolumeTexture()) cp_shader_rendering->SetUniformTexture3D("TexVolume", m_ext_data_manager->GetCurrentVolumeTexture()->GetTextureID(), 1);
  if (m_glsl_transfer_function) cp_shader_rendering->SetUniformTexture1D("TexTransferFunc", m_glsl_transfer_function->GetTextureID(), 2);
  if (m_apply_gradient_shading && m_ext_data_manager->GetCurrentGradientTexture())
    cp_shader_rendering->SetUniformTexture3D("TexVolumeGradient", m_ext_data_manager->GetCurrentGradientTexture()->GetTextureID(), 3);

  cp_shader_rendering->BindUniforms();

  cp_shader_rendering->Unbind();

  ////////////////////////////////////////////////
  // Object Space Mode
  if (cp_lightcache_shader != nullptr)
    delete cp_lightcache_shader;
  cp_lightcache_shader = nullptr;

  if (m_pre_illum_str_vol.IsActive())
  {
    // Initialize compute shader
    cp_lightcache_shader = new gl::ComputeShader();
    cp_lightcache_shader->SetShaderFile(CPPVOLREND_DIR"structured/rc1pvctsg/lightcachecomputation.comp");
    cp_lightcache_shader->LoadAndLink();
    cp_lightcache_shader->Bind();

    cp_lightcache_shader->Unbind();
  }
}

void RC1PVoxelConeTracingSGPU::DestroyRenderingShaders ()
{
  if (cp_shader_rendering)
    delete cp_shader_rendering;
  cp_shader_rendering = NULL;

  gl::ExitOnGLError("Could not destroy the shaders!");
}