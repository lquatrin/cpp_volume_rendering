#include "../../defines.h"
#include "ebsrenderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <vis_utils/camera.h>

#include <volvis_utils/utils.h>
#include <gl_utils/computeshader.h>

#include <random>

/////////////////////////////////
// public functions
/////////////////////////////////
RC1PExtinctionBasedShading::RC1PExtinctionBasedShading ()
  : m_glsl_transfer_function(nullptr)
  , m_u_step_size(0.5f)
  , m_apply_gradient_shading(false)
  , glsl_sat3d_tex(nullptr)
  , transfer_function_changed(false)
{

  //////////////////////////////////////////
  // Occlusion
  apply_ambient_occlusion = true;
  ambient_occlusion_shells = 15;
  ambient_occlusion_radius = 1.0f;

  //////////////////////////////////////////
  // Shadow
  apply_directional_shadows = true;
  dir_shadow_cone_samples = 120;
  dir_shadow_cone_angle = 1.0f;
  dir_shadow_sample_interval = 2.0f;
  dir_shadow_initial_step = 2.0f;
  dir_shadow_user_interface_weight = 1.0f;
  dir_cone_max_distance = 0.0f;
  type_of_shadow = 0;

  //////////////////////////////////////////
  // GLSL Light Cache Texture
  m_pre_illum_str_vol.SetActive(false);
  m_pre_illum_str_vol.SetLightCacheResolution(32, 32, 32);
  cp_lightcache_shader = nullptr;

#ifdef MULTISAMPLE_AVAILABLE
  vr_pixel_multiscaling_support = true;
#endif
}

RC1PExtinctionBasedShading::~RC1PExtinctionBasedShading ()
{
  Clean();
}

void RC1PExtinctionBasedShading::Clean ()
{
  if (m_glsl_transfer_function) delete m_glsl_transfer_function;
  m_glsl_transfer_function = nullptr;

  m_pre_illum_str_vol.DestroyLightCacheTexture();

  if (cp_lightcache_shader != nullptr)
    delete cp_lightcache_shader;
  cp_lightcache_shader = nullptr;

  DestroyRenderingShaders();

  DestroySummedAreaTable();

  BaseVolumeRenderer::Clean();
}

void RC1PExtinctionBasedShading::ReloadShaders ()
{
  cp_shader_rendering->Reload();
  if (cp_lightcache_shader) cp_lightcache_shader->Reload();
}

bool RC1PExtinctionBasedShading::Init (int swidth, int sheight)
{
  if (IsBuilt()) Clean();

  if (m_ext_data_manager->GetCurrentVolumeTexture() == nullptr) return false;
  m_glsl_transfer_function = m_ext_data_manager->GetCurrentTransferFunction()->GenerateTexture_1D_RGBt();

  // Summed Area Table Dimensions 3D
  st_w = m_ext_data_manager->GetCurrentStructuredVolume()->GetWidth();
  st_h = m_ext_data_manager->GetCurrentStructuredVolume()->GetHeight();
  st_d = m_ext_data_manager->GetCurrentStructuredVolume()->GetDepth();
  glsl_sat3d_tex = GenerateExtinctionSAT3DTex(m_ext_data_manager->GetCurrentStructuredVolume(),
                                              m_ext_data_manager->GetCurrentTransferFunction());

  // Get the current Diagonal of the Volume
  vis::StructuredGridVolume* vold = m_ext_data_manager->GetCurrentStructuredVolume();
  float v_w = vold->GetWidth()  * vold->GetScaleX();
  float v_h = vold->GetHeight() * vold->GetScaleY();
  float v_d = vold->GetDepth()  * vold->GetScaleZ();
  float Dv = glm::sqrt(v_w * v_w + v_h * v_h + v_d * v_d);
  // Update the current evaluated distance
  dir_cone_max_distance = 0.75f * Dv;
  
  m_pre_illum_str_vol.GenerateLightCacheTexture();

  CreateRenderingPass();
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

bool RC1PExtinctionBasedShading::Update (vis::Camera* camera)
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

    cp_shader_rendering->SetUniformTexture3D("TexVolumeSAT3D", glsl_sat3d_tex->GetTextureID(), 4);
    cp_shader_rendering->BindUniform("TexVolumeSAT3D");

    cp_shader_rendering->SetUniform("u_sat_width", (int)glsl_sat3d_tex->GetWidth());
    cp_shader_rendering->BindUniform("u_sat_width");
    cp_shader_rendering->SetUniform("u_sat_height", (int)glsl_sat3d_tex->GetHeight());
    cp_shader_rendering->BindUniform("u_sat_height");
    cp_shader_rendering->SetUniform("u_sat_depth", (int)glsl_sat3d_tex->GetDepth());
    cp_shader_rendering->BindUniform("u_sat_depth");

    // Ambient Occlusion
    cp_shader_rendering->SetUniform("AmbOccShells", ambient_occlusion_shells);
    cp_shader_rendering->BindUniform("AmbOccShells");

    cp_shader_rendering->SetUniform("AmbOccRadius", ambient_occlusion_radius);
    cp_shader_rendering->BindUniform("AmbOccRadius");

    // Directional Shadows
    cp_shader_rendering->SetUniform("DirSdwConeSamples", dir_shadow_cone_samples);
    cp_shader_rendering->BindUniform("DirSdwConeSamples");

    cp_shader_rendering->SetUniform("DirSdwConeAngle", (float)(dir_shadow_cone_angle * glm::pi<double>() / 180.0));
    cp_shader_rendering->BindUniform("DirSdwConeAngle");

    cp_shader_rendering->SetUniform("DirSdwSampleInterval", dir_shadow_sample_interval);
    cp_shader_rendering->BindUniform("DirSdwSampleInterval");

    cp_shader_rendering->SetUniform("DirSdwInitialStep", dir_shadow_initial_step);
    cp_shader_rendering->BindUniform("DirSdwInitialStep");

    cp_shader_rendering->SetUniform("DirSdwUserInterfaceWeight", dir_shadow_user_interface_weight);
    cp_shader_rendering->BindUniform("DirSdwUserInterfaceWeight");

    cp_shader_rendering->SetUniform("DirSdwConeMaxDistance", dir_cone_max_distance);
    cp_shader_rendering->BindUniform("DirSdwConeMaxDistance");

    cp_shader_rendering->SetUniform("LightCamForward", m_ext_rendering_parameters->GetBlinnPhongLightSourceCameraForward());
    cp_shader_rendering->BindUniform("LightCamForward");

    cp_shader_rendering->SetUniform("TypeOfShadow", type_of_shadow);
    cp_shader_rendering->BindUniform("TypeOfShadow");
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

  cp_shader_rendering->SetUniform("ApplyShadow", apply_directional_shadows ? 1 : 0);
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

void RC1PExtinctionBasedShading::Redraw ()
{
  m_rdr_frame_to_screen.ClearTexture();

  cp_shader_rendering->Bind();
  m_rdr_frame_to_screen.BindImageTexture();

  cp_shader_rendering->Dispatch();
  gl::ComputeShader::Unbind();

  m_rdr_frame_to_screen.Draw();
}

void RC1PExtinctionBasedShading::MultiSampleRedraw ()
{
  m_rdr_frame_to_screen.ClearTexture();

  cp_shader_rendering->Bind();
  m_rdr_frame_to_screen.BindImageTexture();

  cp_shader_rendering->Dispatch();
  gl::ComputeShader::Unbind();

  m_rdr_frame_to_screen.DrawMultiSampleHigherResolutionMode();
}

void RC1PExtinctionBasedShading::DownScalingRedraw ()
{
  m_rdr_frame_to_screen.ClearTexture();

  cp_shader_rendering->Bind();
  m_rdr_frame_to_screen.BindImageTexture();

  cp_shader_rendering->Dispatch();
  gl::ComputeShader::Unbind();

  m_rdr_frame_to_screen.DrawHigherResolutionWithDownScale();
}

void RC1PExtinctionBasedShading::UpScalingRedraw ()
{
  m_rdr_frame_to_screen.ClearTexture();

  cp_shader_rendering->Bind();
  m_rdr_frame_to_screen.BindImageTexture();

  cp_shader_rendering->Dispatch();
  gl::ComputeShader::Unbind();

  m_rdr_frame_to_screen.DrawLowerResolutionWithUpScale();
}

void RC1PExtinctionBasedShading::SetImGuiComponents ()
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

  ImGui::PushID("Extinction Coefficient Summed Area Table 3D");
  ImGui::Text("- Extinction Coefficient SAT3D Resolution:");
  ImGui::BeginGroup();
  ImGui::InputInt("###ExtCoefVolSAT3DW", &st_w);
  ImGui::InputInt("###ExtCoefVolSAT3DH", &st_h);
  ImGui::InputInt("###ExtCoefVolSAT3DD", &st_d);
  if (ImGui::Button("Update SAT3D Resolution"))
  {
    DestroySummedAreaTable();
    glsl_sat3d_tex = GenerateExtinctionSAT3DTex(m_ext_data_manager->GetCurrentStructuredVolume(),
                                                m_ext_data_manager->GetCurrentTransferFunction());

    SetOutdated();
  }
  ImGui::EndGroup();
  ImGui::PopID();

  ImGui::PushID("Cone Occlusion Data");
  ImGui::Text("- Occlusion Parameters");
  if (ImGui::Checkbox("Apply Ambient Occlusion", &apply_ambient_occlusion))
  {
    SetOutdated();
  }

  if (apply_ambient_occlusion)
  {
    ImGui::Text("Number of Shells");
    if (ImGui::DragInt("###OcclusionNumberOfShells", &ambient_occlusion_shells, 1, 0, 10000))
    {
      SetOutdated();
    }
    ImGui::Text("Occlusion Shell Radius");
    if (ImGui::DragFloat("###OcclusionShellRadius", &ambient_occlusion_radius, 1.0f, 0.0f, 10000.0f))
    {
      SetOutdated();
    }
  }
  ImGui::PopID();

  ImGui::PushID("Cone Shadow Data");
  ImGui::Text("- Shadow Parameters");
  if (ImGui::Checkbox("Apply Shadow", &apply_directional_shadows))
  {
    SetOutdated();
  }
  if (apply_directional_shadows)
  {
    ImGui::Text("Cone Aperture Angle");
    if (ImGui::DragFloat("###ShadowConeAperture", &dir_shadow_cone_angle, 0.5f, 0.5f, 89.5f))
    {
      SetOutdated();
    }

    ImGui::Text("Initial GapStep");
    if (ImGui::DragFloat("###ShadowInitialGapStep", &dir_shadow_initial_step, 0.01f, 0.01f, 10.0f))
    {
      SetOutdated();
    }

    ImGui::Text("Sample Interval");
    if (ImGui::DragFloat("###ShadowSampleInterval", &dir_shadow_sample_interval, 1.0f, 1.0f, 10000.0f))
    {
      SetOutdated();
    }

    ImGui::Text("Covered Distance");
    if (ImGui::DragFloat("###ShadowCoveredDistance", &dir_cone_max_distance, 1.0f, 10.0f, 10000.0f))
    {
      SetOutdated();
    }

    ImGui::Text("Type of Shadow");
    static const char* items_typelightsource[]{ "Point Light", "Directional Light" };
    if (ImGui::Combo("###CurrentShadowTypeOfLightSource", &type_of_shadow,
      items_typelightsource, IM_ARRAYSIZE(items_typelightsource)))
    {
      SetOutdated();
    }
  
    ImGui::Text("Shadow Strength");
    if (ImGui::DragFloat("###ShadowStrengthDrag", &dir_shadow_user_interface_weight, 0.05f, 0.0f, 1000.0f))
    {
      SetOutdated();
    }
  }
  ImGui::PopID();
}

void RC1PExtinctionBasedShading::FillParameterSpace(ParameterSpace& pspace)
{
  pspace.ClearParameterDimensions();
  pspace.AddParameterDimension(new ParameterRangeInt("AmbientOccShells", &ambient_occlusion_shells, 1, 20, 1));
  pspace.AddParameterDimension(new ParameterRangeFloat("AmbientOccRadius", &ambient_occlusion_radius, 0.1f, 1.5f, 0.1f));
}


/////////////////////////////////
// protected/private functions
/////////////////////////////////
void RC1PExtinctionBasedShading::PreComputeLightCache (vis::Camera* camera)
{
  vis::StructuredGridVolume* vol = m_ext_data_manager->GetCurrentStructuredVolume();

  // Initialize compute shader
  cp_lightcache_shader->Bind();

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

  // Bind Summed Area Table
  cp_lightcache_shader->SetUniformTexture3D("TexVolumeSAT3D", glsl_sat3d_tex->GetTextureID(), 3);
  cp_lightcache_shader->BindUniform("TexVolumeSAT3D");

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

  // Ambient Occlusion
  cp_lightcache_shader->SetUniform("AmbOccShells", ambient_occlusion_shells);
  cp_lightcache_shader->BindUniform("AmbOccShells");

  cp_lightcache_shader->SetUniform("AmbOccRadius", ambient_occlusion_radius);
  cp_lightcache_shader->BindUniform("AmbOccRadius");

  // Directional Shadows
  cp_lightcache_shader->SetUniform("DirSdwConeSamples", dir_shadow_cone_samples);
  cp_lightcache_shader->BindUniform("DirSdwConeSamples");

  cp_lightcache_shader->SetUniform("DirSdwConeAngle", (float)(dir_shadow_cone_angle * glm::pi<double>() / 180.0));
  cp_lightcache_shader->BindUniform("DirSdwConeAngle");

  cp_lightcache_shader->SetUniform("DirSdwSampleInterval", dir_shadow_sample_interval);
  cp_lightcache_shader->BindUniform("DirSdwSampleInterval");

  cp_lightcache_shader->SetUniform("DirSdwInitialStep", dir_shadow_initial_step);
  cp_lightcache_shader->BindUniform("DirSdwInitialStep");

  cp_lightcache_shader->SetUniform("DirSdwUserInterfaceWeight", dir_shadow_user_interface_weight);
  cp_lightcache_shader->BindUniform("DirSdwUserInterfaceWeight");

  cp_lightcache_shader->SetUniform("DirSdwConeMaxDistance", dir_cone_max_distance);
  cp_lightcache_shader->BindUniform("DirSdwConeMaxDistance");

  cp_lightcache_shader->SetUniform("LightCamForward", m_ext_rendering_parameters->GetBlinnPhongLightSourceCameraForward());
  cp_lightcache_shader->BindUniform("LightCamForward");

  cp_lightcache_shader->SetUniform("TypeOfShadow", type_of_shadow);
  cp_lightcache_shader->BindUniform("TypeOfShadow");

  // Upload eye position
  cp_lightcache_shader->SetUniform("WorldEyePos", camera->GetEye());
  cp_lightcache_shader->BindUniform("WorldEyePos");

  // Upload light position
  cp_lightcache_shader->SetUniform("WorldLightingPos", m_ext_rendering_parameters->GetBlinnPhongLightingPosition());
  cp_lightcache_shader->BindUniform("WorldLightingPos");

  cp_lightcache_shader->SetUniform("ApplyOcclusion", apply_ambient_occlusion ? 1 : 0);
  cp_lightcache_shader->BindUniform("ApplyOcclusion");

  cp_lightcache_shader->SetUniform("ApplyShadow", apply_directional_shadows ? 1 : 0);
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

void RC1PExtinctionBasedShading::CreateRenderingPass ()
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
    cp_shader_rendering->SetShaderFile(CPPVOLREND_DIR"structured/_common_shaders/obj_ray_marching.comp");
  else
    cp_shader_rendering->SetShaderFile(CPPVOLREND_DIR"structured/rc1pextbsd/ebs_ray_bbox_marching.comp");
  
  cp_shader_rendering->LoadAndLink();
  cp_shader_rendering->Bind();

  cp_shader_rendering->SetUniform("VolumeScales", vol_voxelsize);
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
    cp_lightcache_shader = new gl::ComputeShader();
    cp_lightcache_shader->SetShaderFile(CPPVOLREND_DIR"structured/rc1pextbsd/lightcachecomputation.comp");
    cp_lightcache_shader->LoadAndLink();
    cp_lightcache_shader->Bind();

    cp_lightcache_shader->BindUniforms();

    cp_lightcache_shader->Unbind();
  }
}

void RC1PExtinctionBasedShading::DestroyRenderingShaders ()
{
  if (cp_shader_rendering != nullptr) delete cp_shader_rendering;
  cp_shader_rendering = nullptr;
}

void RC1PExtinctionBasedShading::DestroySummedAreaTable ()
{
  if (glsl_sat3d_tex != nullptr)
    delete glsl_sat3d_tex;
  glsl_sat3d_tex = nullptr;
}

gl::Texture3D* RC1PExtinctionBasedShading::GenerateExtinctionSAT3DTex (vis::StructuredGridVolume* vol, vis::TransferFunction* tf)
{
  // 1
   // First, sample the initial "grid" and build SAT
  int sat_w = (vol->GetWidth() + 2);
  int sat_h = (vol->GetHeight() + 2);
  int sat_d = (vol->GetDepth() + 2);
  
  double min_value = +9999;
  double max_value = -9999;

  vis::SummedAreaTable3D<double> sat3d(sat_w, sat_h, sat_d);
  for (int x = 0; x < sat_w; x++)
  {
    for (int y = 0; y < sat_h; y++)
    {
      for (int z = 0; z < sat_d; z++)
      {
        double val;
        // Adding borders to handle precision issues
        //
        // 0 0 0 0 0 0     0 S S S S S
        // 0         0     0         S
        // 0         0 --> 0         S
        // 0         0     0         S
        // 0 0 0 0 0 0     0 0 0 0 0 0
        //
        if (x == 0 || y == 0 || z == 0 || x == sat_w - 1 || y == sat_h - 1 || z == sat_d - 1)
          val = 0.0f;
        else
        {
          val = tf->GetExtN(vol->GetNormalizedSample(x - 1, y - 1, z - 1));
          min_value = std::min(min_value, val);
          max_value = std::max(max_value, val);
        }
        sat3d.SetValue(val, x, y, z);
      }
    }
  }
  sat3d.BuildSAT();
  printf("SAT min %.2lf max %.2lf\n", min_value, max_value);

  //for (int x = 0; x < sat_w; x++)
  //{
  //  for (int y = 0; y < sat_h; y++)
  //  {
  //    for (int z = 0; z < sat_d; z++)
  //    {
  //      double val;
  //      if (x == 0 || y == 0 || z == 0 || x == sat_w - 1 || y == sat_h - 1 || z == sat_d - 1)
  //        val = 0.0f;
  //      else
  //      {
  //        val = tf->GetExtN(vol->GetNormalizedSample(x - 1, y - 1, z - 1));
  //
  //        double V1 = sat3d.GetValue(x    , y    , z    );
  //        double V2 = sat3d.GetValue(x - 1, y    , z    );
  //        double V3 = sat3d.GetValue(x    , y    , z - 1);
  //        double V4 = sat3d.GetValue(x - 1, y    , z - 1);
  //
  //        double V5 = sat3d.GetValue(x    , y - 1, z    );
  //        double V6 = sat3d.GetValue(x - 1, y - 1, z    );
  //        double V7 = sat3d.GetValue(x    , y - 1, z - 1);
  //        double V8 = sat3d.GetValue(x - 1, y - 1, z - 1);
  //
  //        double val_sat = (V1 - V2 - V3 + V4 - V5 + V6 + V7 - V8);
  //        if (fabs(val_sat - val) > 0.001f)
  //        {
  //          std::cout << "diffffff: " << val << " " << val_sat << std::endl;
  //        }
  //      }
  //    }
  //  }
  //}

  // 2
  // Then, we must create and generate the 3D texture
  double* sat_data = sat3d.GetData();
  GLfloat* data_sat = new GLfloat[sat_w * sat_h * sat_d];
  for (int x = 0; x < sat_w; x++)
  {
    for (int y = 0; y < sat_h; y++)
    {
      for (int z = 0; z < sat_d; z++)
      {
        int id = x + y * sat_w + z * sat_w * sat_h;
        data_sat[id] = (GLfloat)sat_data[id];// / double(x * y * z);
      }
    }
  }

  gl::Texture3D* tex3d_sat = new gl::Texture3D(sat_w, sat_h, sat_d);
  tex3d_sat->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
  tex3d_sat->SetData((GLvoid*)data_sat, GL_R32F, GL_RED, GL_FLOAT);

  delete[] data_sat;

  gl::ExitOnGLError("volrend/utils.cpp - GenerateExtinctionSAT3DTex()");
  return tex3d_sat;
}