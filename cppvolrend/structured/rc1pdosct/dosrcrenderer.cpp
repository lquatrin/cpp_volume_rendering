/**
 * Cone Occlusion based on Gaussian Integrals .cpp
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#include "../../defines.h"
#include "dosrcrenderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <vis_utils/camera.h>

#include <volvis_utils/utils.h>
#include <math_utils/utils.h>

#include <gl_utils/sphere.h>
#include <ctime>

#ifndef DEGREE_TO_RADIANS
#define DEGREE_TO_RADIANS(s) (s * (glm::pi<double>() / 180.0))
#endif

/////////////////////////////////
// public functions
RC1PConeTracingDirOcclusionShading::RC1PConeTracingDirOcclusionShading ()
  : m_glsl_transfer_function(nullptr)
  , cp_shader_rendering(nullptr)
  , m_u_step_size(0.5f)
  , m_apply_gradient_shading(false)
{
  time_vol_generator = 0.0;

  //////////////////////////////////////////
  // Extinction Coefficient Volume
  bind_volume_of_gaussians = true;
  glsl_ext_coef_volume = nullptr;

  // Cone Occlusion
  bind_cone_occlusion_vars = true;
  glsl_apply_occlusion = true;
  glsl_occ_sectionsinfo = nullptr;

  sampler_occlusion.SetUIWeightPercentage(0.350f);
  sampler_occlusion.SetConeHalfAngle(20.0);
  sampler_occlusion.SetMaxGaussianPacking(ConeGaussianSampler::CONEPACKING::_3);

  // Cone Shadow
  bind_cone_shadow_vars = true;
  glsl_apply_shadow = false;
  glsl_sdw_sectionsinfo = nullptr;
  
  sampler_shadow.SetUIWeightPercentage(1.0f);
  sampler_shadow.SetConeHalfAngle(0.5);
  sampler_shadow.SetMaxGaussianPacking(ConeGaussianSampler::CONEPACKING::_1);
  type_of_shadow = 0;

  //////////////////////////////////////////
  // Light Computation Mode
  m_pre_illum_str_vol.SetActive(false);
  m_pre_illum_str_vol.SetLightCacheResolution(32, 32, 32);
  cp_lightcache_shader = nullptr;

#ifdef MULTISAMPLE_AVAILABLE
  vr_pixel_multiscaling_support = true;
#endif
}

RC1PConeTracingDirOcclusionShading::~RC1PConeTracingDirOcclusionShading ()
{
  Clean();
}

void RC1PConeTracingDirOcclusionShading::Clean ()
{
  if (m_glsl_transfer_function) delete m_glsl_transfer_function;
  m_glsl_transfer_function = nullptr;

  // Light cache texture and shader
  m_pre_illum_str_vol.DestroyLightCacheTexture();
  DestroyPreIlluminationShader();

  DestroyRenderingPass();

  DestroyExtCoefVolume();
  DestroyConeSamples();

  BaseVolumeRenderer::Clean();
}

void RC1PConeTracingDirOcclusionShading::ReloadShaders ()
{
  cp_shader_rendering->Reload();
  if (cp_lightcache_shader) cp_lightcache_shader->Reload();
  m_rdr_frame_to_screen.ClearShaders();
}

bool RC1PConeTracingDirOcclusionShading::Init (int swidth, int sheight)
{
  if (IsBuilt()) Clean();

  if (m_ext_data_manager->GetCurrentVolumeTexture() == nullptr) return false;
  m_glsl_transfer_function = m_ext_data_manager->GetCurrentTransferFunction()->GenerateTexture_1D_RGBt();

  CreateRenderingPass();
  gl::ExitOnGLError("Error on Preparing Models and Shaders");

  vis::StructuredGridVolume* vol = m_ext_data_manager->GetCurrentStructuredVolume();
  sampler_occlusion.SetCoveredDistance(vol->GetDiagonal() * 0.50f);
  sampler_shadow.SetCoveredDistance(vol->GetDiagonal() * 0.75f);

  m_pre_illum_str_vol.GenerateLightCacheTexture();

  GenerateExtCoefVolume();
  GenerateConeSamples();

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

bool RC1PConeTracingDirOcclusionShading::Update (vis::Camera* camera)
{
  if (m_pre_illum_str_vol.IsActive())
  {
    PreComputeLightCache(camera);

    cp_shader_rendering->Bind();

    cp_shader_rendering->SetUniformTexture3D("TexVolumeLightCache", m_pre_illum_str_vol.GetLightCacheTexturePointer()->GetTextureID(), 4);
    cp_shader_rendering->BindUniform("TexVolumeLightCache");
  }
  else
  {
    cp_shader_rendering->Bind();


    // Extinction Volume Bindings
    BindExtinctionCoefficientVolume();

    // Representative Cone Uniforms
    BindConeOcclusionUniforms();
    BindConeShadowUniforms();

    /////////////////////////////////////////////////////////////////
    // when computing shadows on the fly!
    cp_shader_rendering->SetUniform("SpotLightMaxAngle", glm::cos(glm::pi<float>() * m_ext_rendering_parameters->GetSpotLightMaxAngle() / 180.f));
    cp_shader_rendering->BindUniform("SpotLightMaxAngle");

    cp_shader_rendering->SetUniform("TypeOfShadow", type_of_shadow);
    cp_shader_rendering->BindUniform("TypeOfShadow");

    /////////////////////////////////////////////////////////////////
    // Light Camera Position vectors
    cp_shader_rendering->SetUniform("LightCamForward", m_ext_rendering_parameters->GetBlinnPhongLightSourceCameraForward());
    cp_shader_rendering->BindUniform("LightCamForward");

    cp_shader_rendering->SetUniform("LightCamUp", m_ext_rendering_parameters->GetBlinnPhongLightSourceCameraUp());
    cp_shader_rendering->BindUniform("LightCamUp");

    cp_shader_rendering->SetUniform("LightCamRight", m_ext_rendering_parameters->GetBlinnPhongLightSourceCameraRight());
    cp_shader_rendering->BindUniform("LightCamRight");
    /////////////////////////////////////////////////////////////////
  }

  cp_shader_rendering->Bind();

  // MULTISAMPLE
  if (IsPixelMultiScalingSupported() && GetCurrentMultiScalingMode() > 0)
  {
    cp_shader_rendering->RecomputeNumberOfGroups(
      m_rdr_frame_to_screen.GetWidth(),
      m_rdr_frame_to_screen.GetHeight(), 0);
  }
  else
  {
    cp_shader_rendering->RecomputeNumberOfGroups(
      m_ext_rendering_parameters->GetScreenWidth(),
      m_ext_rendering_parameters->GetScreenHeight(), 0);
  }

  cp_shader_rendering->SetUniform("CameraEye", camera->GetEye());
  cp_shader_rendering->BindUniform("CameraEye");

  cp_shader_rendering->SetUniform("ViewMatrix", camera->LookAt());
  cp_shader_rendering->BindUniform("ViewMatrix");

  cp_shader_rendering->SetUniform("ProjectionMatrix", camera->Projection());
  cp_shader_rendering->BindUniform("ProjectionMatrix");

  cp_shader_rendering->SetUniform("fov_y_tangent", (float)tan(DEGREE_TO_RADIANS(camera->GetFovY()) / 2.0));
  cp_shader_rendering->BindUniform("fov_y_tangent");

  cp_shader_rendering->SetUniform("aspect_ratio", camera->GetAspectRatio());
  cp_shader_rendering->BindUniform("aspect_ratio");

  cp_shader_rendering->SetUniform("ApplyOcclusion", glsl_apply_occlusion ? 1 : 0);
  cp_shader_rendering->BindUniform("ApplyOcclusion");

  cp_shader_rendering->SetUniform("ApplyShadow", glsl_apply_shadow ? 1 : 0);
  cp_shader_rendering->BindUniform("ApplyShadow");

  cp_shader_rendering->SetUniform("Shade", (glsl_apply_shadow | glsl_apply_occlusion) ? 1 : 0);
  cp_shader_rendering->BindUniform("Shade");

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

void RC1PConeTracingDirOcclusionShading::Redraw ()
{
  m_rdr_frame_to_screen.ClearTexture();

  cp_shader_rendering->Bind();
  m_rdr_frame_to_screen.BindImageTexture();

  cp_shader_rendering->Dispatch();
  gl::ComputeShader::Unbind();

  m_rdr_frame_to_screen.Draw();
}

void RC1PConeTracingDirOcclusionShading::MultiSampleRedraw ()
{
  m_rdr_frame_to_screen.ClearTexture();

  cp_shader_rendering->Bind();
  m_rdr_frame_to_screen.BindImageTexture();

  cp_shader_rendering->Dispatch();
  gl::ComputeShader::Unbind();

  m_rdr_frame_to_screen.DrawMultiSampleHigherResolutionMode();
}

void RC1PConeTracingDirOcclusionShading::DownScalingRedraw ()
{
  m_rdr_frame_to_screen.ClearTexture();

  cp_shader_rendering->Bind();
  m_rdr_frame_to_screen.BindImageTexture();

  cp_shader_rendering->Dispatch();
  gl::ComputeShader::Unbind();

  m_rdr_frame_to_screen.DrawHigherResolutionWithDownScale();
}

void RC1PConeTracingDirOcclusionShading::UpScalingRedraw ()
{
  m_rdr_frame_to_screen.ClearTexture();

  cp_shader_rendering->Bind();
  m_rdr_frame_to_screen.BindImageTexture();

  cp_shader_rendering->Dispatch();
  gl::ComputeShader::Unbind();

  m_rdr_frame_to_screen.DrawLowerResolutionWithUpScale();
}

void RC1PConeTracingDirOcclusionShading::SetImGuiComponents ()
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
    DestroyRenderingPass();
    CreateRenderingPass();
  }
  else if (ret_lc.y)
  {
    SetOutdated();
  }

  ImGui::PushID("Extinction Coefficient Volume");
  ImGui::Text("- Extinction Coefficient Volume:");
  if (ImGui::Checkbox("Use Custom Volume", ext_coef_vol_gen.IsUsingCustomExtCoefVolumeResolutionPtr()))
  {
    GenerateExtCoefVolume();
  }
  if (ext_coef_vol_gen.IsUsingCustomExtCoefVolumeResolution())
  {
    ImGui::BeginGroup();
    ImGui::InputInt("###ExtCoefVolCustomW", &ext_coef_vol_gen.GetCustomExtCoefVolumeResolutionPtr()->x);
    ImGui::InputInt("###ExtCoefVolCustomH", &ext_coef_vol_gen.GetCustomExtCoefVolumeResolutionPtr()->y);
    ImGui::InputInt("###ExtCoefVolCustomD", &ext_coef_vol_gen.GetCustomExtCoefVolumeResolutionPtr()->z);
    if (ImGui::Button("Update Extinction Volume"))
    {
      GenerateExtCoefVolume();
      SetOutdated();
    }
    ImGui::EndGroup();
  }
  ImGui::Text("Sigme at base level");
  ImGui::BeginGroup();
  ImGui::InputFloat("###Sigma0Input", ext_coef_vol_gen.GetBaseLevelGaussianSigma0Ptr());
  if (ImGui::Button("Update Sigma_0"))
  {
    sampler_occlusion.SetInitialStep(3.0f*ext_coef_vol_gen.GetBaseLevelGaussianSigma0());
    sampler_shadow.SetInitialStep(3.0f*ext_coef_vol_gen.GetBaseLevelGaussianSigma0());
    GenerateExtCoefVolume();
    GenerateConeSamples();
    SetOutdated();
  }
  ImGui::EndGroup();
  //ImGui::Text("Generation Time: %.4f", (float)time_vol_generator);
  ImGui::PopID();

  ImGui::PushID("Cone Occlusion Data");
  ImGui::Text("- Occlusion Parameters");
  if (ImGui::Checkbox("Apply Occlusion", &glsl_apply_occlusion))
  {
    SetOutdated();
  }
  if (glsl_apply_occlusion)
  {
    ImGui::Text("Cone Aperture Angle");
    if (ImGui::DragFloat("###OcclusionConeAperture", &sampler_occlusion.cone_half_angle, 0.5f, 0.5f, 89.5f))
    {
      bind_cone_occlusion_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }

    ImGui::Text("Initial GapStep");
    if (ImGui::DragFloat("###OcclusionInitialGapStep", &sampler_occlusion.initial_step, 0.01f, 0.01f, 10.0f))
    {
      bind_cone_occlusion_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }

    ImGui::Text("Cone Sample Separation Multiplier");
    if (ImGui::DragFloat("###OcclusionConeSampleSeparation", &sampler_occlusion.d_sigma, 0.01f, 0.01f, 10.0f))
    {
      bind_cone_occlusion_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }

    ImGui::Text("Minimum Circular Cone Section");
    if (ImGui::DragFloat("###OcclusionMinimumCircularConeSection", &sampler_occlusion.r_sigma, 0.01f, 0.01f, 10.0f))
    {
      bind_cone_occlusion_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }

    ImGui::Text("Covered Distance");
    if (ImGui::DragFloat("###OcclusionCoveredDistance", &sampler_occlusion.covered_distance, 1.0f, 10.0f, 10000.0f))
    {
      bind_cone_occlusion_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }

    ImGui::Text("Cone Split");
    ImGui::SameLine();
    int e = sampler_occlusion.GetMaxGaussianPackingInt();
    if (ImGui::RadioButton("1###OccMaxPack1", &e, 0))
    {
      sampler_occlusion.SetMaxGaussianPacking(ConeGaussianSampler::CONEPACKING::_1);
      bind_cone_occlusion_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("3###OccMaxPack3", &e, 1))
    {
      sampler_occlusion.SetMaxGaussianPacking(ConeGaussianSampler::CONEPACKING::_3);
      bind_cone_occlusion_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("7###OccMaxPack7", &e, 2))
    {
      sampler_occlusion.SetMaxGaussianPacking(ConeGaussianSampler::CONEPACKING::_7);
      bind_cone_occlusion_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }

    ImGui::Text("Attenuation Factor");
    if (ImGui::DragFloat("###OcclusionAttenuationFactor", &sampler_occlusion.ui_weight_percentage, 0.001f, 0.0f, 1.0f))
    {
      bind_cone_occlusion_vars = true;
      SetOutdated();
    }
  }
  ImGui::PopID();

  ImGui::PushID("Cone Shadow Data");
  ImGui::Text("- Shadow Parameters");
  if (ImGui::Checkbox("Apply Shadow", &glsl_apply_shadow))
  {
    SetOutdated();
  }
  if (glsl_apply_shadow)
  {
    ImGui::Text("Cone Aperture Angle");
    if (ImGui::DragFloat("###ShadowConeAperture", &sampler_shadow.cone_half_angle, 0.5f, 0.5f, 89.5f))
    {
      bind_cone_shadow_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }

    ImGui::Text("Initial GapStep");
    if (ImGui::DragFloat("###ShadowInitialGapStep", &sampler_shadow.initial_step, 0.01f, 0.01f, 10.0f))
    {
      bind_cone_shadow_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }

    ImGui::Text("Cone Sample Separation Multiplier");
    if (ImGui::DragFloat("###ShadowConeSampleSeparation", &sampler_shadow.d_sigma, 0.01f, 0.01f, 10.0f))
    {
      bind_cone_shadow_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }

    ImGui::Text("Minimum Circular Cone Section");
    if (ImGui::DragFloat("###ShadowMinimumCircularConeSection", &sampler_shadow.r_sigma, 0.01f, 0.01f, 10.0f))
    {
      bind_cone_shadow_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }

    ImGui::Text("Covered Distance");
    if (ImGui::DragFloat("###ShadowCoveredDistance", &sampler_shadow.covered_distance, 1.0f, 10.0f, 10000.0f))
    {
      bind_cone_shadow_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }

    ImGui::Text("Cone Split");
    ImGui::SameLine();
    int e = sampler_shadow.GetMaxGaussianPackingInt();
    if (ImGui::RadioButton("1###SdwMaxPack1", &e, 0))
    {
      sampler_shadow.SetMaxGaussianPacking(ConeGaussianSampler::CONEPACKING::_1);
      bind_cone_shadow_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("3###SdwMaxPack3", &e, 1))
    {
      sampler_shadow.SetMaxGaussianPacking(ConeGaussianSampler::CONEPACKING::_3);
      bind_cone_shadow_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("7###SdwMaxPack7", &e, 2))
    {
      sampler_shadow.SetMaxGaussianPacking(ConeGaussianSampler::CONEPACKING::_7);
      bind_cone_shadow_vars = true;
      GenerateConeSamples();
      SetOutdated();
    }

    ImGui::Text("Attenuation Factor");
    if (ImGui::DragFloat("###ShadowAttenuationFactor", &sampler_shadow.ui_weight_percentage, 0.001f, 0.0f, 1.0f))
    {
      bind_cone_shadow_vars = true;
      SetOutdated();
    }

    ImGui::Text("Type of Shadow");
    static const char* items_typelightsource[]{ "Point Light", "Spot Light", "Directional Light" };
    if (ImGui::Combo("###CurrentShadowTypeOfLightSource", &type_of_shadow,
      items_typelightsource, IM_ARRAYSIZE(items_typelightsource)))
    {
      SetOutdated();
    }
  }
  ImGui::PopID();
}

/////////////////////////////////
// protected/private functions
void RC1PConeTracingDirOcclusionShading::PreComputeLightCache (vis::Camera* camera)
{
  cp_lightcache_shader->Bind();

  vis::StructuredGridVolume* vol = m_ext_data_manager->GetCurrentStructuredVolume();

  glm::vec3 vol_scale = glm::vec3(m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleX(),
    m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleY(),
    m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleZ());

  glActiveTexture(GL_TEXTURE0);

  int w = m_ext_data_manager->GetCurrentStructuredVolume()->GetWidth();
  int h = m_ext_data_manager->GetCurrentStructuredVolume()->GetHeight();
  int d = m_ext_data_manager->GetCurrentStructuredVolume()->GetDepth();

  // Bind our input texture 3D
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

  cp_lightcache_shader->SetUniformTexture3D("TexVolume", m_ext_data_manager->GetCurrentVolumeTexture()->GetTextureID(), 1);
  cp_lightcache_shader->BindUniform("TexVolume");

  cp_lightcache_shader->SetUniformTexture1D("TexTransferFunc", m_glsl_transfer_function->GetTextureID(), 2);
  cp_lightcache_shader->BindUniform("TexTransferFunc");

  cp_lightcache_shader->SetUniform("ApplyOcclusion", glsl_apply_occlusion ? 1 : 0);
  cp_lightcache_shader->BindUniform("ApplyOcclusion");

  cp_lightcache_shader->SetUniform("ApplyShadow", glsl_apply_shadow ? 1 : 0);
  cp_lightcache_shader->BindUniform("ApplyShadow");

  // Upload eye position
  cp_lightcache_shader->SetUniform("WorldEyePos", camera->GetEye());
  cp_lightcache_shader->BindUniform("WorldEyePos");

  // Upload light position
  cp_lightcache_shader->SetUniform("WorldLightingPos", m_ext_rendering_parameters->GetBlinnPhongLightingPosition());
  cp_lightcache_shader->BindUniform("WorldLightingPos");

  BindExtinctionCoefficientVolume();
  BindConeOcclusionUniforms();
  BindConeShadowUniforms();

  /////////////////////////////////////////////////////////////////
  // when computing shadows on the fly!
  cp_lightcache_shader->SetUniform("SpotLightMaxAngle", glm::cos(glm::pi<float>() * m_ext_rendering_parameters->GetSpotLightMaxAngle() / 180.f));
  cp_lightcache_shader->BindUniform("SpotLightMaxAngle");

  cp_lightcache_shader->SetUniform("TypeOfShadow", type_of_shadow);
  cp_lightcache_shader->BindUniform("TypeOfShadow");

  /////////////////////////////////////////////////////////////////
  // Light Camera Position vectors
  cp_lightcache_shader->SetUniform("LightCamForward", m_ext_rendering_parameters->GetBlinnPhongLightSourceCameraForward());
  cp_lightcache_shader->BindUniform("LightCamForward");

  cp_lightcache_shader->SetUniform("LightCamUp", m_ext_rendering_parameters->GetBlinnPhongLightSourceCameraUp());
  cp_lightcache_shader->BindUniform("LightCamUp");

  cp_lightcache_shader->SetUniform("LightCamRight", m_ext_rendering_parameters->GetBlinnPhongLightSourceCameraRight());
  cp_lightcache_shader->BindUniform("LightCamRight");

  /////////////////////////////////////////////////////////////////
  // Eye Camera Position vectors
  glm::vec3 v_dir, v_up, v_right;
  camera->GetCameraVectors(&v_dir, &v_up, &v_right);
  cp_lightcache_shader->SetUniform("EyeCamForward", v_dir);
  cp_lightcache_shader->BindUniform("EyeCamForward");

  cp_lightcache_shader->SetUniform("EyeCamUp", v_up);
  cp_lightcache_shader->BindUniform("EyeCamUp");

  cp_lightcache_shader->SetUniform("EyeCamRight", v_right);
  cp_lightcache_shader->BindUniform("EyeCamRight");

  glActiveTexture(GL_TEXTURE0);
  cp_lightcache_shader->RecomputeNumberOfGroups(
    m_pre_illum_str_vol.GetLightCacheTexturePointer()->GetWidth(),
    m_pre_illum_str_vol.GetLightCacheTexturePointer()->GetHeight(),
    m_pre_illum_str_vol.GetLightCacheTexturePointer()->GetDepth());

  cp_lightcache_shader->Dispatch();

  glBindTexture(GL_TEXTURE_3D, 0);
  glActiveTexture(GL_TEXTURE0);

  cp_lightcache_shader->Unbind();

  gl::ExitOnGLError("ERROR: After SetData");
}

void RC1PConeTracingDirOcclusionShading::CreateRenderingPass ()
{
  bind_volume_of_gaussians = true;
  bind_cone_occlusion_vars = true;
  bind_cone_shadow_vars = true;

  glm::vec3 vol_aabb = glm::vec3(
    float(m_ext_data_manager->GetCurrentStructuredVolume()->GetWidth())  * (m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleX()),
    float(m_ext_data_manager->GetCurrentStructuredVolume()->GetHeight()) * (m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleY()),
    float(m_ext_data_manager->GetCurrentStructuredVolume()->GetDepth())  * (m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleZ())
  );
    
  cp_shader_rendering = new gl::ComputeShader();

  if (m_pre_illum_str_vol.IsActive())
    cp_shader_rendering->SetShaderFile(CPPVOLREND_DIR"structured/_common_shaders/obj_ray_marching.comp");
  else
    cp_shader_rendering->SetShaderFile(CPPVOLREND_DIR"structured/rc1pdosct/ray_bbox_marching.comp");

  cp_shader_rendering->LoadAndLink();
  cp_shader_rendering->Bind();
  
  cp_shader_rendering->SetUniform("VolumeScaledSizes", vol_aabb);
  
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
    cp_lightcache_shader->SetShaderFile(CPPVOLREND_DIR"structured/rc1pdosct/lightcachecomputation.comp");
    cp_lightcache_shader->LoadAndLink();
    cp_lightcache_shader->Bind();

    vis::StructuredGridVolume* vol = m_ext_data_manager->GetCurrentStructuredVolume();

    //// Bind volume texture
    //cp_lightcache_shader->SetUniformTexture3D("TexVolume", glsl_volume->GetTextureID(), 0);
    //cp_lightcache_shader->BindUniform("TexVolume");
    //
    //// Bind transfer function texture
    //cp_lightcache_shader->SetUniformTexture1D("TexTransferFunc", glsl_transfer_function->GetTextureID(), 1);
    //cp_lightcache_shader->BindUniform("TexTransferFunc");

    // Upload volume dimensions
    glm::vec3 volsize(vol->GetWidth(), vol->GetHeight(), vol->GetDepth());
    cp_lightcache_shader->SetUniform("VolumeDimensions", volsize);
    cp_lightcache_shader->BindUniform("VolumeDimensions");

    // Upload volume scales
    cp_lightcache_shader->SetUniform("VolumeScales", glm::vec3(vol->GetScale()));
    cp_lightcache_shader->BindUniform("VolumeScales");

    // Upload volume scaled sizes
    glm::vec3 volssize(vol->GetWidth()  * vol->GetScaleX(),
      vol->GetHeight() * vol->GetScaleY(),
      vol->GetDepth()  * vol->GetScaleZ());
    cp_lightcache_shader->SetUniform("VolumeScaledSizes", volssize);
    cp_lightcache_shader->BindUniform("VolumeScaledSizes");

    cp_lightcache_shader->Unbind();
  }
}

void RC1PConeTracingDirOcclusionShading::DestroyRenderingPass ()
{
  if (cp_shader_rendering) delete cp_shader_rendering;
  cp_shader_rendering = nullptr;

  gl::ExitOnGLError("Could not destroy shaders!");
}

void RC1PConeTracingDirOcclusionShading::GenerateExtCoefVolume ()
{
  DestroyExtCoefVolume();

  GLuint64 startTime, stopTime;
  unsigned int queryID[2];

  glsl_ext_coef_volume = ext_coef_vol_gen.BuildMipMappedTexture(
    m_ext_data_manager->GetCurrentVolumeTexture(),
    m_ext_data_manager->GetCurrentTransferFunction()->GenerateTexture_1D_RGBA(),
    glm::vec3(m_ext_data_manager->GetCurrentStructuredVolume()->GetScale()));

  // request binding of extinction coefficient volume
  bind_volume_of_gaussians = true;

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, 0);

  SetOutdated();
}

void RC1PConeTracingDirOcclusionShading::DestroyExtCoefVolume ()
{
  if (glsl_ext_coef_volume) delete glsl_ext_coef_volume;
  glsl_ext_coef_volume = nullptr;

  gl::ExitOnGLError("Could not destroy gaussian data!");
}

void RC1PConeTracingDirOcclusionShading::GenerateConeSamples ()
{
  DestroyConeSamples();

  GLuint64 startTime, stopTime;
  unsigned int queryID[2];

  sampler_occlusion.ComputeConeIntegrationSteps(ext_coef_vol_gen.GetBaseLevelGaussianSigma0());
  glsl_occ_sectionsinfo = sampler_occlusion.GetConeSectionsInfoTex();

  bind_cone_occlusion_vars = true;

  sampler_shadow.ComputeConeIntegrationSteps(ext_coef_vol_gen.GetBaseLevelGaussianSigma0());
  glsl_sdw_sectionsinfo = sampler_shadow.GetConeSectionsInfoTex();

  bind_cone_shadow_vars = true;

  SetOutdated();
}

void RC1PConeTracingDirOcclusionShading::DestroyConeSamples ()
{
  if (glsl_occ_sectionsinfo) delete glsl_occ_sectionsinfo;
  glsl_occ_sectionsinfo = nullptr;

  if (glsl_sdw_sectionsinfo) delete glsl_sdw_sectionsinfo;
  glsl_sdw_sectionsinfo = nullptr;

  gl::ExitOnGLError("Could not destroy sampling data!");
}

void RC1PConeTracingDirOcclusionShading::BindExtinctionCoefficientVolume ()
{
  if (m_pre_illum_str_vol.IsActive())
  {
    cp_lightcache_shader->SetUniformTexture3D("TexVolumeOfGaussians", glsl_ext_coef_volume->GetTextureID(), 3);
    cp_lightcache_shader->BindUniform("TexVolumeOfGaussians");
  } 
  else
  {
    if (bind_volume_of_gaussians)
    {
      cp_shader_rendering->SetUniformTexture3D("TexVolumeOfGaussians", glsl_ext_coef_volume->GetTextureID(), 4);
      cp_shader_rendering->BindUniform("TexVolumeOfGaussians");
    }
    bind_volume_of_gaussians = false;
  }
}

void RC1PConeTracingDirOcclusionShading::BindConeOcclusionUniforms ()
{
  if (m_pre_illum_str_vol.IsActive())
  {
    cp_lightcache_shader->SetUniformTexture1D("TexOccConeSectionsInfo", glsl_occ_sectionsinfo->GetTextureID(), 4);
    cp_lightcache_shader->BindUniform("TexOccConeSectionsInfo");

    cp_lightcache_shader->SetUniform("OccInitialStep", (float)sampler_occlusion.GetInitialStep());
    cp_lightcache_shader->BindUniform("OccInitialStep");

    cp_lightcache_shader->SetUniform("OccRay7AdjWeight", (float)sampler_occlusion.GetRay7AdjacentWeight());
    cp_lightcache_shader->BindUniform("OccRay7AdjWeight");

    std::vector<glm::vec3> occ_cone_axes;
    occ_cone_axes.push_back(sampler_occlusion.Get3ConeRayID(0));
    occ_cone_axes.push_back(sampler_occlusion.Get3ConeRayID(1));
    occ_cone_axes.push_back(sampler_occlusion.Get3ConeRayID(2));
    occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(0));
    occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(1));
    occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(2));
    occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(3));
    occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(4));
    occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(5));
    occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(6));

    cp_lightcache_shader->SetUniformArray("OccConeRayAxes", occ_cone_axes);
    cp_lightcache_shader->BindUniform("OccConeRayAxes");

    std::vector<int> cone_integration_steps;
    cone_integration_steps.push_back(sampler_occlusion.gaussian_samples_1);
    cone_integration_steps.push_back(sampler_occlusion.gaussian_samples_3);
    cone_integration_steps.push_back(sampler_occlusion.gaussian_samples_7);

    cp_lightcache_shader->SetUniformArray("OccConeIntegrationSamples", cone_integration_steps);
    cp_lightcache_shader->BindUniform("OccConeIntegrationSamples");

    cp_lightcache_shader->SetUniform("OccUIWeight", (float)sampler_occlusion.ui_weight_percentage);
    cp_lightcache_shader->BindUniform("OccUIWeight");
  }
  else
  {
    if (bind_cone_occlusion_vars)
    {
      cp_shader_rendering->SetUniformTexture1D("TexOccConeSectionsInfo", glsl_occ_sectionsinfo->GetTextureID(), 5);
      cp_shader_rendering->BindUniform("TexOccConeSectionsInfo");

      cp_shader_rendering->SetUniform("OccInitialStep", (float)sampler_occlusion.GetInitialStep());
      cp_shader_rendering->BindUniform("OccInitialStep");

      cp_shader_rendering->SetUniform("OccRay7AdjWeight", (float)sampler_occlusion.GetRay7AdjacentWeight());
      cp_shader_rendering->BindUniform("OccRay7AdjWeight");

      std::vector<glm::vec3> occ_cone_axes;
      occ_cone_axes.push_back(sampler_occlusion.Get3ConeRayID(0));
      occ_cone_axes.push_back(sampler_occlusion.Get3ConeRayID(1));
      occ_cone_axes.push_back(sampler_occlusion.Get3ConeRayID(2));
      occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(0));
      occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(1));
      occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(2));
      occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(3));
      occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(4));
      occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(5));
      occ_cone_axes.push_back(sampler_occlusion.Get7ConeRayID(6));

      cp_shader_rendering->SetUniformArray("OccConeRayAxes", occ_cone_axes);
      cp_shader_rendering->BindUniform("OccConeRayAxes");

      std::vector<int> cone_integration_steps;
      cone_integration_steps.push_back(sampler_occlusion.gaussian_samples_1);
      cone_integration_steps.push_back(sampler_occlusion.gaussian_samples_3);
      cone_integration_steps.push_back(sampler_occlusion.gaussian_samples_7);

      cp_shader_rendering->SetUniformArray("OccConeIntegrationSamples", cone_integration_steps);
      cp_shader_rendering->BindUniform("OccConeIntegrationSamples");

      cp_shader_rendering->SetUniform("OccUIWeight", (float)sampler_occlusion.ui_weight_percentage);
      cp_shader_rendering->BindUniform("OccUIWeight");
    }
    bind_cone_occlusion_vars = false;
  }
}

void RC1PConeTracingDirOcclusionShading::BindConeShadowUniforms ()
{
  if (m_pre_illum_str_vol.IsActive())
  {
    cp_lightcache_shader->SetUniformTexture1D("TexSdwConeSectionsInfo", glsl_sdw_sectionsinfo->GetTextureID(), 5);
    cp_lightcache_shader->BindUniform("TexSdwConeSectionsInfo");

    cp_lightcache_shader->SetUniform("SdwInitialStep", (float)sampler_shadow.GetInitialStep());
    cp_lightcache_shader->BindUniform("SdwInitialStep");

    cp_lightcache_shader->SetUniform("SdwRay7AdjWeight", (float)sampler_shadow.GetRay7AdjacentWeight());
    cp_lightcache_shader->BindUniform("SdwRay7AdjWeight");

    std::vector<glm::vec3> sdw_cone_axes;
    sdw_cone_axes.push_back(sampler_shadow.Get3ConeRayID(0));
    sdw_cone_axes.push_back(sampler_shadow.Get3ConeRayID(1));
    sdw_cone_axes.push_back(sampler_shadow.Get3ConeRayID(2));
    sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(0));
    sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(1));
    sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(2));
    sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(3));
    sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(4));
    sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(5));
    sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(6));

    cp_lightcache_shader->SetUniformArray("SdwConeRayAxes", sdw_cone_axes);
    cp_lightcache_shader->BindUniform("SdwConeRayAxes");

    std::vector<int> cone_integration_steps_sdw;
    cone_integration_steps_sdw.push_back(sampler_shadow.gaussian_samples_1);
    cone_integration_steps_sdw.push_back(sampler_shadow.gaussian_samples_3);
    cone_integration_steps_sdw.push_back(sampler_shadow.gaussian_samples_7);

    cp_lightcache_shader->SetUniformArray("SdwConeIntegrationSamples", cone_integration_steps_sdw);
    cp_lightcache_shader->BindUniform("SdwConeIntegrationSamples");

    cp_lightcache_shader->SetUniform("SdwUIWeight", (float)sampler_shadow.ui_weight_percentage);
    cp_lightcache_shader->BindUniform("SdwUIWeight");
  }
  else
  {
    if (bind_cone_shadow_vars)
    {
      cp_shader_rendering->SetUniformTexture1D("TexSdwConeSectionsInfo", glsl_sdw_sectionsinfo->GetTextureID(), 6);
      cp_shader_rendering->BindUniform("TexSdwConeSectionsInfo");

      cp_shader_rendering->SetUniform("SdwInitialStep", (float)sampler_shadow.GetInitialStep());
      cp_shader_rendering->BindUniform("SdwInitialStep");

      cp_shader_rendering->SetUniform("SdwRay7AdjWeight", (float)sampler_shadow.GetRay7AdjacentWeight());
      cp_shader_rendering->BindUniform("SdwRay7AdjWeight");

      std::vector<glm::vec3> sdw_cone_axes;
      sdw_cone_axes.push_back(sampler_shadow.Get3ConeRayID(0));
      sdw_cone_axes.push_back(sampler_shadow.Get3ConeRayID(1));
      sdw_cone_axes.push_back(sampler_shadow.Get3ConeRayID(2));
      sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(0));
      sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(1));
      sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(2));
      sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(3));
      sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(4));
      sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(5));
      sdw_cone_axes.push_back(sampler_shadow.Get7ConeRayID(6));

      cp_shader_rendering->SetUniformArray("SdwConeRayAxes", sdw_cone_axes);
      cp_shader_rendering->BindUniform("SdwConeRayAxes");

      std::vector<int> cone_integration_steps_sdw;
      cone_integration_steps_sdw.push_back(sampler_shadow.gaussian_samples_1);
      cone_integration_steps_sdw.push_back(sampler_shadow.gaussian_samples_3);
      cone_integration_steps_sdw.push_back(sampler_shadow.gaussian_samples_7);

      cp_shader_rendering->SetUniformArray("SdwConeIntegrationSamples", cone_integration_steps_sdw);
      cp_shader_rendering->BindUniform("SdwConeIntegrationSamples");

      cp_shader_rendering->SetUniform("SdwUIWeight", (float)sampler_shadow.ui_weight_percentage);
      cp_shader_rendering->BindUniform("SdwUIWeight");
    }
    bind_cone_shadow_vars = false;
  }
}

void RC1PConeTracingDirOcclusionShading::DestroyPreIlluminationShader ()
{
  if (cp_lightcache_shader != nullptr)
    delete cp_lightcache_shader;
  cp_lightcache_shader = nullptr;
}