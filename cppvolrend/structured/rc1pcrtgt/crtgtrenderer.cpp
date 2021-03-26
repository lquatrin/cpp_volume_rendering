/**
 * Cone Occlusion based on Ray casting .cpp
 * . used as our "Ground Truth"
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#include "../../defines.h"
#include "crtgtrenderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <vis_utils/camera.h>
#include <math_utils/utils.h>

#include <random>
#include <ctime>
#include <chrono>

RC1PConeLightGroundTruthSteps::RC1PConeLightGroundTruthSteps ()
  : m_gt_rendering(nullptr)
  , m_rgba_screen_data(nullptr)
  , m_rgba_state_fragment(nullptr)
  , m_cp_rendering(nullptr)
  , m_tex_transfer_function(nullptr)
  , m_tex_gt_state(nullptr)
  , m_frame_outdated(true)
  , m_show_frame_texture(false)
  , m_u_step_size(0.5f)
  , m_apply_gradient(false)
{
  m_u_light_ray_initial_step = 1.0f;
  m_u_light_ray_step_size = 0.5f;

  m_light_parameters_outdated = true;

  m_apply_occlusion = false;
  m_occ_num_rays_sampled = 1;
  m_occ_cone_aperture_angle = 90.f;
  m_occ_tex_raysampled_vectors = nullptr;
  m_occ_cone_distance_eval = 100.0f;

  m_apply_shadows = false;
  m_sdw_num_rays_sampled = 1;
  m_sdw_cone_aperture_angle = 1.0f;
  m_sdw_tex_raysampled_vectors = nullptr;
  m_sdw_cone_distance_eval = 100.0f;
  m_shadow_type = 0;

#ifdef MULTISAMPLE_AVAILABLE
  vr_pixel_multiscaling_support = true;
#endif
}

RC1PConeLightGroundTruthSteps::~RC1PConeLightGroundTruthSteps ()
{
  Clean();
}

void RC1PConeLightGroundTruthSteps::Clean ()
{
  DestroyIntegrationPass();
  
  if (m_rgba_screen_data) delete m_rgba_screen_data;
  m_rgba_screen_data = nullptr;

  if (m_rgba_state_fragment) delete m_rgba_state_fragment;
  m_rgba_state_fragment = nullptr;
  
  if (m_tex_gt_state) delete m_tex_gt_state;
  m_tex_gt_state = nullptr;

  if (m_tex_transfer_function) delete m_tex_transfer_function;
  m_tex_transfer_function = nullptr;

  if (m_occ_tex_raysampled_vectors) delete m_occ_tex_raysampled_vectors;
  m_occ_tex_raysampled_vectors = nullptr;

  if (m_sdw_tex_raysampled_vectors) delete m_sdw_tex_raysampled_vectors;
  m_sdw_tex_raysampled_vectors = nullptr;
  
  BaseVolumeRenderer::Clean();
}

void RC1PConeLightGroundTruthSteps::ReloadShaders()
{
  m_cp_rendering->Reload();
  m_gt_rendering->Reload();
}

bool RC1PConeLightGroundTruthSteps::Init (int shader_width, int shader_height)
{
  if (IsBuilt()) Clean();

  if (m_ext_data_manager->GetCurrentVolumeTexture() == nullptr) return false;
  m_tex_transfer_function = m_ext_data_manager->GetCurrentTransferFunction()->GenerateTexture_1D_RGBt();

  CreateIntegrationPass();
  gl::ExitOnGLError("RC1PCPURenderer: Error on creating rendering pass.");

  Reshape(shader_width, shader_height);

  // estimate initial integration step
  glm::dvec3 sv = m_ext_data_manager->GetCurrentStructuredVolume()->GetScale();
  m_u_step_size = float((0.5 / glm::sqrt(3.0)) * glm::sqrt(sv.x * sv.x + sv.y * sv.y + sv.z * sv.z));

  if (m_ext_data_manager->GetCurrentStructuredVolume())
  {
    m_occ_cone_distance_eval = m_ext_data_manager->GetCurrentStructuredVolume()->GetDiagonal() * 0.50f;
    m_sdw_cone_distance_eval = m_ext_data_manager->GetCurrentStructuredVolume()->GetDiagonal() * 0.75f;
  }

  SetBuilt(true);
  SetOutdated();
  return true;
}

bool RC1PConeLightGroundTruthSteps::Update (vis::Camera* camera)
{
  // if the chamera is changing, reset the frame texture
  if (camera->Changing())
    m_show_frame_texture = false;

  if(IsOutdated())
  {
    if (m_light_parameters_outdated)
    {
      std::default_random_engine generator;
      std::uniform_real_distribution<float> distribution(0.0, 1.0);

      // reference vector (cone ray direction)
      glm::vec3 gtray = glm::vec3(0, 0, 1);

      // Occlusion
      ////////////////////////////////////////////////////////////////////////////////////////
      GLfloat* occ_kernel_vectors = new GLfloat[m_occ_num_rays_sampled * 3];
      for (int i = 0; i < m_occ_num_rays_sampled; i++)
      {
        float theta_angle = distribution(generator) * (glm::pi<float>() * (m_occ_cone_aperture_angle / 180.f));
        float phi_angle   = distribution(generator) * (glm::pi<float>() * (360.f                     / 180.f));
      
        glm::vec3 ray_sample_vec = RodriguesRotation(gtray, theta_angle, glm::vec3(0, 1, 0));
        ray_sample_vec = RodriguesRotation(ray_sample_vec, phi_angle, gtray);
      
        occ_kernel_vectors[(i * 3) + 0] = ray_sample_vec.x;
        occ_kernel_vectors[(i * 3) + 1] = ray_sample_vec.y;
        occ_kernel_vectors[(i * 3) + 2] = ray_sample_vec.z;
      }
      
      if (m_occ_tex_raysampled_vectors != nullptr) delete m_occ_tex_raysampled_vectors;
      m_occ_tex_raysampled_vectors = nullptr;
      
      m_occ_tex_raysampled_vectors = new gl::Texture1D(m_occ_num_rays_sampled);
      m_occ_tex_raysampled_vectors->GenerateTexture(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);
      m_occ_tex_raysampled_vectors->SetData(occ_kernel_vectors, GL_RGB16F, GL_RGB, GL_FLOAT);
      delete[] occ_kernel_vectors;
      ////////////////////////////////////////////////////////////////////////////////////////

      // Shadow
      ////////////////////////////////////////////////////////////////////////////////////////
      GLfloat* sdw_kernel_vectors = new GLfloat[m_sdw_num_rays_sampled * 3];
      for (int i = 0; i < m_sdw_num_rays_sampled; i++)
      {
        float theta_angle = distribution(generator) * (glm::pi<float>() * (m_sdw_cone_aperture_angle / 180.f));
        float phi_angle = distribution(generator) * (glm::pi<float>() * (360.f / 180.f));

        glm::vec3 ray_sample_vec = RodriguesRotation(gtray, theta_angle, glm::vec3(0, 1, 0));
        ray_sample_vec = RodriguesRotation(ray_sample_vec, phi_angle, gtray);

        sdw_kernel_vectors[(i * 3) + 0] = ray_sample_vec.x;
        sdw_kernel_vectors[(i * 3) + 1] = ray_sample_vec.y;
        sdw_kernel_vectors[(i * 3) + 2] = ray_sample_vec.z;
      }

      if (m_sdw_tex_raysampled_vectors != nullptr) delete m_sdw_tex_raysampled_vectors;
      m_sdw_tex_raysampled_vectors = nullptr;

      m_sdw_tex_raysampled_vectors = new gl::Texture1D(m_sdw_num_rays_sampled);
      m_sdw_tex_raysampled_vectors->GenerateTexture(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);
      m_sdw_tex_raysampled_vectors->SetData(sdw_kernel_vectors, GL_RGB16F, GL_RGB, GL_FLOAT);
      delete[] sdw_kernel_vectors;

      m_light_parameters_outdated = false;
    }

    m_gt_rendering->Bind();

    m_gt_rendering->SetUniform("CameraEye", camera->GetEye());
    m_gt_rendering->SetUniform("CameraLookAt", camera->LookAt());
    m_gt_rendering->SetUniform("CameraProjection", camera->Projection());
    m_gt_rendering->SetUniform("CameraAspectRatio", camera->GetAspectRatio());
    m_gt_rendering->SetUniform("TanCameraFovY", (float)glm::tan((camera->GetFovY() / 2.0) * glm::pi<double>() / 180.0));

    m_gt_rendering->SetUniform("StepSize", m_u_step_size);

    m_gt_rendering->SetUniform("LightRayInitialGap", m_u_light_ray_initial_step);
    m_gt_rendering->SetUniform("LightRayStepSize", m_u_light_ray_step_size);
    
    m_gt_rendering->SetUniform("ApplyConeOcclusion", m_apply_occlusion ? 1 : 0);
    m_gt_rendering->SetUniform("OccNumberOfSampledRays", m_occ_num_rays_sampled);
    m_gt_rendering->SetUniform("OccConeApertureAngle", m_occ_cone_aperture_angle);
    m_gt_rendering->SetUniform("OccConeDistanceEvaluation", m_occ_cone_distance_eval);
    if (m_occ_tex_raysampled_vectors)
      m_gt_rendering->SetUniformTexture1D("TexOccRaysSampledVectors", m_occ_tex_raysampled_vectors->GetTextureID(), 5);
    
    m_gt_rendering->SetUniform("ApplyConeShadow", m_apply_shadows ? 1 : 0);
    m_gt_rendering->SetUniform("SdwNumberOfSampledRays", m_sdw_num_rays_sampled);
    m_gt_rendering->SetUniform("SdwConeApertureAngle", m_sdw_cone_aperture_angle);
    m_gt_rendering->SetUniform("SdwConeDistanceEvaluation", m_sdw_cone_distance_eval);
    m_gt_rendering->SetUniform("SdwShadowType", m_shadow_type);
    if (m_sdw_tex_raysampled_vectors)
      m_gt_rendering->SetUniformTexture1D("TexSdwRaysSampledVectors", m_sdw_tex_raysampled_vectors->GetTextureID(), 6);

    bool use_grad = (m_ext_data_manager->GetCurrentGradientTexture() != nullptr) && m_apply_gradient;
    m_gt_rendering->SetUniform("ApplyGradientPhongShading", use_grad ? 1 : 0);
    
    glm::vec3 lpos = camera->GetEye();
    glm::vec3 zaxs = camera->GetZAxis();
    glm::vec3 yaxs = camera->GetYAxis();
    glm::vec3 xaxs = camera->GetXAxis();
    if (m_ext_rendering_parameters->GetNumberOfLightSources() > 0)
    {
      lpos = m_ext_rendering_parameters->GetBlinnPhongLightingPosition();
      zaxs = -m_ext_rendering_parameters->GetBlinnPhongLightSourceCameraForward();
      yaxs = m_ext_rendering_parameters->GetBlinnPhongLightSourceCameraUp();
      xaxs = m_ext_rendering_parameters->GetBlinnPhongLightSourceCameraRight();
    }
    m_gt_rendering->SetUniform("LightSourcePosition", lpos);
    m_gt_rendering->SetUniform("LightCamForward", zaxs);
    m_gt_rendering->SetUniform("LightCamUp", yaxs);
    m_gt_rendering->SetUniform("LightCamRight", xaxs);

    m_gt_rendering->SetUniform("BlinnPhongKa", m_ext_rendering_parameters->GetBlinnPhongKambient());
    m_gt_rendering->SetUniform("BlinnPhongKd", m_ext_rendering_parameters->GetBlinnPhongKdiffuse());
    m_gt_rendering->SetUniform("BlinnPhongKs", m_ext_rendering_parameters->GetBlinnPhongKspecular());
    m_gt_rendering->SetUniform("BlinnPhongShininess", m_ext_rendering_parameters->GetBlinnPhongNshininess());

    m_gt_rendering->BindUniforms();
    gl::ComputeShader::Unbind();

    ///////////////////////////////////////////////////////////////////////////////

    m_cp_rendering->Bind();

    m_cp_rendering->SetUniform("CameraEye", camera->GetEye());
    m_cp_rendering->SetUniform("CameraLookAt", camera->LookAt());
    m_cp_rendering->SetUniform("CameraProjection", camera->Projection());
    m_cp_rendering->SetUniform("CameraAspectRatio", camera->GetAspectRatio());
    m_cp_rendering->SetUniform("TanCameraFovY", (float)glm::tan((camera->GetFovY() / 2.0) * glm::pi<double>() / 180.0));

    m_cp_rendering->BindUniforms();
    gl::ComputeShader::Unbind();
  
    vr_outdated = false;
  }
  return true;
}

void RC1PConeLightGroundTruthSteps::PreRedraw ()
{
  if (!m_show_frame_texture)
  {
    m_rdr_frame_to_screen.ClearTextureImage();
    if (m_tex_gt_state)
      glClearTexImage(m_tex_gt_state->GetTextureID(), 0, GL_RG, GL_FLOAT, 0);
  }
}

void RC1PConeLightGroundTruthSteps::RedrawFrameTexture ()
{
  if (m_frame_outdated)
  {
    if (m_rgba_screen_data == nullptr)
    {
      m_rgba_screen_data = new GLfloat[m_rdr_frame_to_screen.GetScreenOutputTexture()->GetWidth() *
                                       m_rdr_frame_to_screen.GetScreenOutputTexture()->GetHeight() * 4];

      if (m_rgba_state_fragment) delete m_rgba_state_fragment;
      m_rgba_state_fragment = new GLfloat[m_rdr_frame_to_screen.GetScreenOutputTexture()->GetWidth() *
                                          m_rdr_frame_to_screen.GetScreenOutputTexture()->GetHeight() * 2];
    }
    m_gt_rendering->Bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_rdr_frame_to_screen.GetScreenOutputTexture()->GetTextureID());
    glBindImageTexture(0, m_rdr_frame_to_screen.GetScreenOutputTexture()->GetTextureID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16F);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_tex_gt_state->GetTextureID());
    glBindImageTexture(1, m_tex_gt_state->GetTextureID(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RG16F);

    glActiveTexture(GL_TEXTURE0);
    m_gt_rendering->Dispatch();
    gl::ComputeShader::Unbind();
    gl::ExitOnGLError("RC1PConeLightGroundTruthSteps: After dispatch to generate a new frame.");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_rdr_frame_to_screen.GetScreenOutputTexture()->GetTextureID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, m_rgba_screen_data);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_tex_gt_state->GetTextureID());
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, m_rgba_state_fragment);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_frame_outdated = false;
    int sx = m_rdr_frame_to_screen.GetScreenOutputTexture()->GetWidth() * m_rdr_frame_to_screen.GetScreenOutputTexture()->GetHeight();
    for (int i = 0; i < sx; i++)
    {
      if (m_rgba_state_fragment[i * 2 + 1] < 0.5f)
      {
        m_frame_outdated = true;
        break;
      }
    }
  }
  else
  {
    m_rdr_frame_to_screen.GetScreenOutputTexture()->SetData(m_rgba_screen_data, GL_RGBA16F, GL_RGBA, GL_FLOAT);
  }
}

void RC1PConeLightGroundTruthSteps::RedrawCube ()
{
  m_cp_rendering->Bind();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_rdr_frame_to_screen.GetScreenOutputTexture()->GetTextureID());
  glBindImageTexture(0, m_rdr_frame_to_screen.GetScreenOutputTexture()->GetTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

  m_cp_rendering->Dispatch();
  gl::ComputeShader::Unbind();
  gl::ExitOnGLError("RC1PConeLightGroundTruthSteps: After dispatch.");
}

void RC1PConeLightGroundTruthSteps::Redraw ()
{
  PreRedraw();

  if (m_show_frame_texture)
    RedrawFrameTexture();
  else
    RedrawCube();
  
  m_rdr_frame_to_screen.Draw();
}

void RC1PConeLightGroundTruthSteps::MultiSampleRedraw ()
{
  PreRedraw();

  if (m_show_frame_texture)
    RedrawFrameTexture();
  else
    RedrawCube();

  m_rdr_frame_to_screen.DrawMultiSampleHigherResolutionMode();
}

void RC1PConeLightGroundTruthSteps::DownScalingRedraw ()
{
  PreRedraw();

  if (m_show_frame_texture)
    RedrawFrameTexture();
  else
    RedrawCube();

  if (m_show_frame_texture && m_frame_outdated)
    m_rdr_frame_to_screen.Draw();
  else
    m_rdr_frame_to_screen.DrawHigherResolutionWithDownScale();
}

void RC1PConeLightGroundTruthSteps::UpScalingRedraw()
{
  PreRedraw();

  if (m_show_frame_texture)
    RedrawFrameTexture();
  else
    RedrawCube();

  if (m_show_frame_texture && m_frame_outdated)
    m_rdr_frame_to_screen.Draw();
  else
    m_rdr_frame_to_screen.DrawLowerResolutionWithUpScale();
}

void RC1PConeLightGroundTruthSteps::Reshape (int w, int h)
{
  BaseVolumeRenderer::Reshape(w, h);

  // delete current rgba screen data
  if (m_rgba_screen_data) delete m_rgba_screen_data;
  m_rgba_screen_data = nullptr;

  if (m_tex_gt_state) delete m_tex_gt_state;
  
  m_tex_gt_state = new gl::Texture2D(m_rdr_frame_to_screen.GetScreenOutputTexture()->GetWidth(),
                                     m_rdr_frame_to_screen.GetScreenOutputTexture()->GetHeight());
  m_tex_gt_state->GenerateTexture(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
  m_tex_gt_state->SetData(NULL, GL_RG16F, GL_RG, GL_FLOAT);

  m_gt_rendering->RecomputeNumberOfGroups(m_rdr_frame_to_screen.GetScreenOutputTexture()->GetWidth(),
                                          m_rdr_frame_to_screen.GetScreenOutputTexture()->GetHeight(), 0);
  m_cp_rendering->RecomputeNumberOfGroups(m_rdr_frame_to_screen.GetScreenOutputTexture()->GetWidth(),
                                          m_rdr_frame_to_screen.GetScreenOutputTexture()->GetHeight(), 0);
  ResetOutputFrameGeneration();
}

void RC1PConeLightGroundTruthSteps::SetImGuiComponents ()
{
  ImGui::PushID("Ground Truth parameters");
  if (m_frame_outdated)
  {
    ImGui::Text("- Frame OutDated");
  }
  else
  {
    ImGui::Text("- Frame Ready");
  }

  if (ImGui::Checkbox("Show Generated Frame Texture###RCGLSLShowFrameTexture", &m_show_frame_texture))
  {
    m_frame_outdated = true;
    SetOutdated();
  }
  if (ImGui::Button("Update Output Frame###RCCPUUpdateOutputFrame"))
  {
    m_frame_outdated = true;
    SetOutdated();
  }

  ImGui::Text("- Step Size: ");
  if (ImGui::DragFloat("###RayCasting1PassUIIntegrationStepSize", &m_u_step_size, 0.01f, 0.01f, 100.0f, "%.2f"))
  {
    m_show_frame_texture = false;
    SetOutdated();
  }

  if (AddImGuiMultiSampleOptions())
  {
    if (m_rgba_screen_data) delete m_rgba_screen_data;
    m_rgba_screen_data = nullptr;

    if (m_tex_gt_state) delete m_tex_gt_state;
  
    m_tex_gt_state = new gl::Texture2D(m_rdr_frame_to_screen.GetScreenOutputTexture()->GetWidth(),
                                       m_rdr_frame_to_screen.GetScreenOutputTexture()->GetHeight());
    m_tex_gt_state->GenerateTexture(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    m_tex_gt_state->SetData(NULL, GL_RG16F, GL_RG, GL_FLOAT);

    m_gt_rendering->RecomputeNumberOfGroups(m_rdr_frame_to_screen.GetScreenOutputTexture()->GetWidth(),
                                            m_rdr_frame_to_screen.GetScreenOutputTexture()->GetHeight(), 0);
    m_cp_rendering->RecomputeNumberOfGroups(m_rdr_frame_to_screen.GetScreenOutputTexture()->GetWidth(),
                                            m_rdr_frame_to_screen.GetScreenOutputTexture()->GetHeight(), 0);
    ResetOutputFrameGeneration();
  }
  
  if (m_ext_data_manager->GetCurrentGradientTexture())
  {
    if (ImGui::Checkbox("Apply Gradient###RayCasting1PassApplyGradient", &m_apply_gradient))
    {
      m_show_frame_texture = false;
      SetOutdated();
    }
  }
  
  ImGui::Text("- Light Ray Initial Gap: ");
  if (ImGui::DragFloat("###RayCasting1PassUIIntegrationLightRayInitialGapStep", &m_u_light_ray_initial_step, 0.01f, 0.01f, 100.0f, "%.2f"))
  {
    m_show_frame_texture = false;
    SetOutdated();
  }
  
  ImGui::Text("- Light Ray Step Size: ");
  if (ImGui::DragFloat("###RayCasting1PassUIIntegrationLightRayStepSize", &m_u_light_ray_step_size, 0.01f, 0.01f, 100.0f, "%.2f"))
  {
    m_show_frame_texture = false;
    SetOutdated();
  }

  // Occlusion
  if (ImGui::Checkbox("Apply Occlusion###RC1PGLSLGTApplyOcclusion", &m_apply_occlusion))
  {
    m_show_frame_texture = false;
    m_light_parameters_outdated = true;
    SetOutdated();
  }
  ImGui::Text("Number of Sampled Rays");
  if (ImGui::DragInt("###OccNumberOfSampledRays", &m_occ_num_rays_sampled, 1, 0, 10000))
  {
    m_show_frame_texture = false;
    m_light_parameters_outdated = true;
    SetOutdated();
  }
  ImGui::Text("Cone Aperture Angle");
  if (ImGui::DragFloat("###OcclusionConeAperture", &m_occ_cone_aperture_angle, 0.5f, 0.0f, 90.0f))
  {
    m_show_frame_texture = false;
    m_light_parameters_outdated = true;
    SetOutdated();
  }
  ImGui::Text("Cone Distance Evaluation");
  if (ImGui::DragFloat("###OcclusionConeDistance", &m_occ_cone_distance_eval, 1.0f, 1.0f, 10000.0f))
  {
    m_show_frame_texture = false;
    m_light_parameters_outdated = true;
    SetOutdated();
  }

  // Shadow
  if (ImGui::Checkbox("Apply Shadow###RC1PGLSLGTApplyShadow", &m_apply_shadows))
  {
    m_show_frame_texture = false;
    m_light_parameters_outdated = true;
    SetOutdated();
  }
  ImGui::Text("Number of Sampled Rays");
  if (ImGui::DragInt("###SdwNumberOfSampledRays", &m_sdw_num_rays_sampled, 1, 0, 10000))
  {
    m_show_frame_texture = false;
    m_light_parameters_outdated = true;
    SetOutdated();
  }
  ImGui::Text("Cone Aperture Angle");
  if (ImGui::DragFloat("###ShadowConeAperture", &m_sdw_cone_aperture_angle, 0.5f, 0.0f, 90.0f))
  {
    m_show_frame_texture = false;
    m_light_parameters_outdated = true;
    SetOutdated();
  }
  ImGui::Text("Cone Distance Evaluation");
  if (ImGui::DragFloat("###ShadowConeDistance", &m_sdw_cone_distance_eval, 1.0f, 1.0f, 10000.0f))
  {
    m_show_frame_texture = false;
    m_light_parameters_outdated = true;
    SetOutdated();
  }
  
  ImGui::Text("Type of Shadow");
  static const char* items_typelightsource[] { "Point Light", "Spot Light", "Directional Light" };
  if (ImGui::Combo("###CurrentShadowTypeOfLightSource", &m_shadow_type,
    items_typelightsource, IM_ARRAYSIZE(items_typelightsource)))
  {
    m_show_frame_texture = false;
    SetOutdated();
  }
  ImGui::PopID();
}

void RC1PConeLightGroundTruthSteps::CreateIntegrationPass ()
{
  vis::StructuredGridVolume* svol = m_ext_data_manager->GetCurrentStructuredVolume();

  m_gt_rendering = new gl::ComputeShader();
  m_gt_rendering->AddShaderFile(CPPVOLREND_DIR"/structured/rc1pcrtgt/gt_ray_marching.comp");
  m_gt_rendering->LoadAndLink();
  m_gt_rendering->Bind();

  m_gt_rendering->SetUniform("VolumeGridSize", glm::vec3(
    float(svol->GetWidth()) * svol->GetScaleX(),
    float(svol->GetHeight()) * svol->GetScaleY(),
    float(svol->GetDepth()) * svol->GetScaleZ()
  ));

  m_gt_rendering->SetUniform("VolumeGridResolution",
    glm::vec3(svol->GetWidth(), svol->GetHeight(), svol->GetDepth())
  );

  m_gt_rendering->SetUniformTexture3D("TexVolume", m_ext_data_manager->GetCurrentVolumeTexture()->GetTextureID(), 2);
  m_gt_rendering->SetUniformTexture1D("TexTransferFunc", m_tex_transfer_function->GetTextureID(), 3);

  if (m_ext_data_manager->GetCurrentGradientTexture())
    m_gt_rendering->SetUniformTexture3D("TexVolumeGradient", m_ext_data_manager->GetCurrentGradientTexture()->GetTextureID(), 4);

  m_gt_rendering->BindUniforms();
  gl::ComputeShader::Unbind();

  m_cp_rendering = new gl::ComputeShader();
  m_cp_rendering->AddShaderFile(CPPVOLREND_DIR"/structured/rc1pcrtgt/vol_intersection.comp");
  m_cp_rendering->LoadAndLink();
  m_cp_rendering->Bind();

  m_cp_rendering->SetUniform("VolumeGridSize", glm::vec3(
    float(svol->GetWidth()) * svol->GetScaleX(),
    float(svol->GetHeight()) * svol->GetScaleY(),
    float(svol->GetDepth()) * svol->GetScaleZ()
  ));

  m_cp_rendering->SetUniform("VolumeGridResolution",
    glm::vec3(svol->GetWidth(), svol->GetHeight(), svol->GetDepth())
  );

  m_cp_rendering->BindUniforms();
  gl::ComputeShader::Unbind();
}

void RC1PConeLightGroundTruthSteps::DestroyIntegrationPass ()
{
  if (m_gt_rendering) delete m_gt_rendering;
  m_gt_rendering = nullptr;

  if (m_cp_rendering) delete m_cp_rendering;
  m_cp_rendering = nullptr;
}

void RC1PConeLightGroundTruthSteps::ResetOutputFrameGeneration ()
{
  m_show_frame_texture = false;
  m_frame_outdated = true;
}
