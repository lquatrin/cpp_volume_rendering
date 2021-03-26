#include "../../defines.h"
#include "sbtmdosrenderer.h"

#include <math_utils/utils.h>
#include <vis_utils/camera.h>

#include <gl_utils/pipelineshader.h>
#include <gl_utils/camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <volvis_utils/utils.h>

SBTMDirectionalOcclusionShading::SBTMDirectionalOcclusionShading ()
  : m_glsl_transfer_function(nullptr)
  , ps_shader_rendering(nullptr)
  , shader_blend_pass(nullptr)
  , m_u_step_size(0.5f)
  , quad_vao(nullptr)
  , quad_vbo(nullptr)
  , quad_ibo(nullptr)
  , lfb_object(3)
{
  volume_diagonal = 0.0;

  proxy_geom.quads.clear();

  shader_width = -1;
  shader_height = -1;

  cone_half_angle = 50.0f;
  evaluate_dir_occlusion = true;
  grid_size = glm::vec2(2.0f);

  max_number_of_passes = 90000;

  occ_ui_weight_percentage = 1.0f;

#ifdef MULTISAMPLE_AVAILABLE
  vr_pixel_multiscaling_support = true;
#endif
}

SBTMDirectionalOcclusionShading::~SBTMDirectionalOcclusionShading ()
{
  Clean();
}

void SBTMDirectionalOcclusionShading::Clean ()
{
  if (m_glsl_transfer_function) delete m_glsl_transfer_function;
  m_glsl_transfer_function = nullptr;

  if (ps_shader_rendering)
    delete ps_shader_rendering;
  ps_shader_rendering = nullptr;

  if (shader_blend_pass)
    delete shader_blend_pass;
  shader_blend_pass = nullptr;

  lfb_object.Unbind();
  lfb_object.DestroyAttachments();

  BaseVolumeRenderer::Clean();
}

void SBTMDirectionalOcclusionShading::ReloadShaders ()
{
  ps_shader_rendering->Reload();
  shader_blend_pass->Reload();
}

bool SBTMDirectionalOcclusionShading::Init (int swidth, int sheight)
{
  if (IsBuilt()) Clean();

  if (m_ext_data_manager->GetCurrentVolumeTexture() == nullptr) return false;
  m_glsl_transfer_function = m_ext_data_manager->GetCurrentTransferFunction()->GenerateTexture_1D_RGBt();

  shader_width = swidth;
  shader_height = sheight;

  ProjectionMatrix = glm::mat4();
  ViewMatrix = glm::mat4();


  lfb_object.GenerateAttachments(shader_width, shader_height);

  vis::StructuredGridVolume* vr = m_ext_data_manager->GetCurrentStructuredVolume();
  double Lx = vr->GetWidth();
  double Ly = vr->GetHeight();
  double Lz = vr->GetDepth();
  volume_diagonal = glm::sqrt(Lx*Lx + Ly * Ly + Lz * Lz);

  CreateSlicePass(m_ext_data_manager->GetCurrentStructuredVolume());
  CreateBlendPass();

  gl::ExitOnGLError("While Preparing Models and Shaders");

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

bool SBTMDirectionalOcclusionShading::Update (vis::Camera* camera)
{

  ViewMatrix = camera->LookAt();
  ProjectionMatrix = camera->Projection();

  cam_eye = camera->GetEye();
  cam_dir = glm::normalize(camera->GetDir());

  ps_shader_rendering->Bind();
  ps_shader_rendering->SetUniform("ViewMatrix", ViewMatrix);
  ps_shader_rendering->BindUniform("ViewMatrix");

  ps_shader_rendering->SetUniform("ProjectionMatrix", ProjectionMatrix);
  ps_shader_rendering->BindUniform("ProjectionMatrix");

  ps_shader_rendering->SetUniform("StepSize", m_u_step_size);
  ps_shader_rendering->BindUniform("StepSize");

  ps_shader_rendering->SetUniform("VolumeDiagonal", (float)volume_diagonal);
  ps_shader_rendering->BindUniform("VolumeDiagonal");

  ////////////////////////////////////////////////////////////////////////
  // Equation 11
  float occlusion_extent = m_u_step_size * glm::tan(cone_half_angle * glm::pi<float>() / 180.0f);
  ps_shader_rendering->SetUniform("OcclusionExtent", occlusion_extent);
  ps_shader_rendering->BindUniform("OcclusionExtent");

  ps_shader_rendering->SetUniform("GridSize", grid_size);
  ps_shader_rendering->BindUniform("GridSize");

  ps_shader_rendering->SetUniform("UITransparencyScale", occ_ui_weight_percentage);
  ps_shader_rendering->BindUniform("UITransparencyScale");

  gl::PipelineShader::Unbind();

  // All vertices of the bounding box
  glm::vec3 aabb = 0.5f * glm::vec3(vol_scale.x * m_ext_data_manager->GetCurrentStructuredVolume()->GetWidth(),
                                    vol_scale.y * m_ext_data_manager->GetCurrentStructuredVolume()->GetHeight(),
                                    vol_scale.z * m_ext_data_manager->GetCurrentStructuredVolume()->GetDepth());

  cam_up = glm::normalize(glm::vec3(0, 1, 0));
  cam_right = glm::normalize(glm::cross(cam_up, -cam_dir));
  cam_up = glm::normalize(glm::cross(-cam_dir, cam_right));

  // compute maximum z value and minimum z value based on used camera
  ComputeMinMaxZBoudingBox(aabb);

  // compute proxy geometry quads
  ComputeProxyGeometry();

  return true;
}

void SBTMDirectionalOcclusionShading::Redraw ()
{
  GLuint occlusion_buffer_next = 0;
  GLuint occlusion_buffer_prev = 1;

  GLuint eye_buffer = 2;

  lfb_object.Bind();

  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);

  GLuint attachmentsi[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, attachmentsi);

  ps_shader_rendering->Bind();

  // https://stackoverflow.com/questions/44756898/opengl-different-clear-color-for-individual-color-attachments
  // Using opengl 4.4+
  glClearTexImage(lfb_object.GetColorAttachmentID(2), 0, GL_RGBA, GL_FLOAT, 0);
  // We don't need to update the other occlusion buffer since the blending is not applied here
  const float init_opacity[4] = { 1, 0, 0, 0 };
  glClearTexImage(lfb_object.GetColorAttachmentID(1), 0, GL_RGBA, GL_FLOAT, &init_opacity);
  glClearTexImage(lfb_object.GetColorAttachmentID(0), 0, GL_RGBA, GL_FLOAT, &init_opacity);

  ps_shader_rendering->SetUniform("CameraDirection", cam_dir);
  ps_shader_rendering->BindUniform("CameraDirection");

  quad_vao->Bind();
  quad_vbo->Bind();
  quad_ibo->Bind();

  // Render each slice using glDrawElements
  number_of_passes = 0;
  for (int i = 0; i < proxy_geom.quads.size() && number_of_passes < max_number_of_passes; i++)
  {
    lfb_object.SetRenderTargetColoAttachment(0, eye_buffer);
    lfb_object.SetRenderTargetColoAttachment(1, occlusion_buffer_next);
    // TODO: change the swap buffer strategy
    // TODO: https://gist.github.com/roxlu/6b0d2081675c24c607d0

    ps_shader_rendering->SetUniformTexture2D("EyeBufferPrev", lfb_object.GetColorAttachmentID(eye_buffer), 2);
    ps_shader_rendering->BindUniform("EyeBufferPrev");

    ps_shader_rendering->SetUniformTexture2D("OcclusionBufferPrev", lfb_object.GetColorAttachmentID(occlusion_buffer_prev), 3);
    ps_shader_rendering->BindUniform("OcclusionBufferPrev");

    ps_shader_rendering->SetUniform("DistanceFromNearSlice", proxy_geom.quads[i].pos_from_near);
    ps_shader_rendering->BindUniform("DistanceFromNearSlice");

    quad_vao->DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT);

    occlusion_buffer_prev = occlusion_buffer_next;
    occlusion_buffer_next = (occlusion_buffer_next + 1) % 2;

    number_of_passes++;
  }

  gl::BufferObject::Unbind(GL_ARRAY_BUFFER);
  gl::BufferObject::Unbind(GL_ELEMENT_ARRAY_BUFFER);
  gl::ArrayObject::Unbind();

  ps_shader_rendering->Unbind();

  glPopAttrib();

  lfb_object.Unbind();
  // BLEND THE RESULT WITH THE BACKGROUND COLOR
  // 27. CompositWithWindowFrameBuffer(eye_buffer)
//#define SIBGRAPI_2019
#ifdef SIBGRAPI_2019
  shader_blend_pass->Bind();
  shader_blend_pass->SetUniformTexture2D("EyeBufferSlice", lfb_object.GetColorAttachmentID(2), 0);
  shader_blend_pass->BindUniform("EyeBufferSlice");
  
  // render quad to composite with screen
  glBegin(GL_QUADS);
  glVertex3f(-1, -1, 0);
  glVertex3f(+1, -1, 0);
  glVertex3f(+1, +1, 0);
  glVertex3f(-1, +1, 0);
  glEnd();
  
  gl::Shader::Unbind();
#else
  m_rdr_frame_to_screen.Draw(lfb_object.GetColorAttachmentID(2));
#endif
}

void SBTMDirectionalOcclusionShading::MultiSampleRedraw ()
{
  GLuint occlusion_buffer_next = 0;
  GLuint occlusion_buffer_prev = 1;

  GLuint eye_buffer = 2;

  lfb_object.Bind();

  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT | GL_VIEWPORT_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glViewport(0,0,shader_width, shader_height);
  GLuint attachmentsi[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, attachmentsi);

  ps_shader_rendering->Bind();

  // https://stackoverflow.com/questions/44756898/opengl-different-clear-color-for-individual-color-attachments
  // Using opengl 4.4+
  glClearTexImage(lfb_object.GetColorAttachmentID(2), 0, GL_RGBA, GL_FLOAT, 0);
  // We don't need to update the other occlusion buffer since the blending is not applied here
  const float init_opacity[4] = { 1, 0, 0, 0 };
  glClearTexImage(lfb_object.GetColorAttachmentID(1), 0, GL_RGBA, GL_FLOAT, &init_opacity);
  glClearTexImage(lfb_object.GetColorAttachmentID(0), 0, GL_RGBA, GL_FLOAT, &init_opacity);

  ps_shader_rendering->SetUniform("CameraDirection", cam_dir);
  ps_shader_rendering->BindUniform("CameraDirection");

  quad_vao->Bind();
  quad_vbo->Bind();
  quad_ibo->Bind();

  // Render each slice using glDrawElements
  number_of_passes = 0;
  for (int i = 0; i < proxy_geom.quads.size() && number_of_passes < max_number_of_passes; i++)
  {
    lfb_object.SetRenderTargetColoAttachment(0, eye_buffer);
    lfb_object.SetRenderTargetColoAttachment(1, occlusion_buffer_next);
    // TODO: change the swap buffer strategy
    // TODO: https://gist.github.com/roxlu/6b0d2081675c24c607d0

    ps_shader_rendering->SetUniformTexture2D("EyeBufferPrev", lfb_object.GetColorAttachmentID(eye_buffer), 2);
    ps_shader_rendering->BindUniform("EyeBufferPrev");

    ps_shader_rendering->SetUniformTexture2D("OcclusionBufferPrev", lfb_object.GetColorAttachmentID(occlusion_buffer_prev), 3);
    ps_shader_rendering->BindUniform("OcclusionBufferPrev");

    ps_shader_rendering->SetUniform("DistanceFromNearSlice", proxy_geom.quads[i].pos_from_near);
    ps_shader_rendering->BindUniform("DistanceFromNearSlice");

    quad_vao->DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT);

    occlusion_buffer_prev = occlusion_buffer_next;
    occlusion_buffer_next = (occlusion_buffer_next + 1) % 2;

    number_of_passes++;
  }

  gl::BufferObject::Unbind(GL_ARRAY_BUFFER);
  gl::BufferObject::Unbind(GL_ELEMENT_ARRAY_BUFFER);
  gl::ArrayObject::Unbind();

  ps_shader_rendering->Unbind();

  glPopAttrib();

  lfb_object.Unbind();
  // BLEND THE RESULT WITH THE BACKGROUND COLOR
  // 27. CompositWithWindowFrameBuffer(eye_buffer)
//#define SIBGRAPI_2019
#ifdef SIBGRAPI_2019
  shader_blend_pass->Bind();
  shader_blend_pass->SetUniformTexture2D("EyeBufferSlice", lfb_object.GetColorAttachmentID(2), 0);
  shader_blend_pass->BindUniform("EyeBufferSlice");

  // render quad to composite with screen
  glBegin(GL_QUADS);
  glVertex3f(-1, -1, 0);
  glVertex3f(+1, -1, 0);
  glVertex3f(+1, +1, 0);
  glVertex3f(-1, +1, 0);
  glEnd();

  gl::Shader::Unbind();
#else
  m_rdr_frame_to_screen.DrawMultiSampleHigherResolutionMode(lfb_object.GetColorAttachmentID(2));
#endif
}

void SBTMDirectionalOcclusionShading::DownScalingRedraw ()
{
  GLuint occlusion_buffer_next = 0;
  GLuint occlusion_buffer_prev = 1;

  GLuint eye_buffer = 2;

  lfb_object.Bind();

  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT | GL_VIEWPORT_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glViewport(0, 0, shader_width, shader_height);
  GLuint attachmentsi[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, attachmentsi);

  ps_shader_rendering->Bind();

  // https://stackoverflow.com/questions/44756898/opengl-different-clear-color-for-individual-color-attachments
  // Using opengl 4.4+
  glClearTexImage(lfb_object.GetColorAttachmentID(2), 0, GL_RGBA, GL_FLOAT, 0);
  // We don't need to update the other occlusion buffer since the blending is not applied here
  const float init_opacity[4] = { 1, 0, 0, 0 };
  glClearTexImage(lfb_object.GetColorAttachmentID(1), 0, GL_RGBA, GL_FLOAT, &init_opacity);
  glClearTexImage(lfb_object.GetColorAttachmentID(0), 0, GL_RGBA, GL_FLOAT, &init_opacity);

  ps_shader_rendering->SetUniform("CameraDirection", cam_dir);
  ps_shader_rendering->BindUniform("CameraDirection");

  quad_vao->Bind();
  quad_vbo->Bind();
  quad_ibo->Bind();

  // Render each slice using glDrawElements
  number_of_passes = 0;
  for (int i = 0; i < proxy_geom.quads.size() && number_of_passes < max_number_of_passes; i++)
  {
    lfb_object.SetRenderTargetColoAttachment(0, eye_buffer);
    lfb_object.SetRenderTargetColoAttachment(1, occlusion_buffer_next);
    // TODO: change the swap buffer strategy
    // TODO: https://gist.github.com/roxlu/6b0d2081675c24c607d0

    ps_shader_rendering->SetUniformTexture2D("EyeBufferPrev", lfb_object.GetColorAttachmentID(eye_buffer), 2);
    ps_shader_rendering->BindUniform("EyeBufferPrev");

    ps_shader_rendering->SetUniformTexture2D("OcclusionBufferPrev", lfb_object.GetColorAttachmentID(occlusion_buffer_prev), 3);
    ps_shader_rendering->BindUniform("OcclusionBufferPrev");

    ps_shader_rendering->SetUniform("DistanceFromNearSlice", proxy_geom.quads[i].pos_from_near);
    ps_shader_rendering->BindUniform("DistanceFromNearSlice");

    quad_vao->DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT);

    occlusion_buffer_prev = occlusion_buffer_next;
    occlusion_buffer_next = (occlusion_buffer_next + 1) % 2;

    number_of_passes++;
  }

  gl::BufferObject::Unbind(GL_ARRAY_BUFFER);
  gl::BufferObject::Unbind(GL_ELEMENT_ARRAY_BUFFER);
  gl::ArrayObject::Unbind();

  ps_shader_rendering->Unbind();

  glPopAttrib();

  lfb_object.Unbind();
  // BLEND THE RESULT WITH THE BACKGROUND COLOR
  // 27. CompositWithWindowFrameBuffer(eye_buffer)
//#define SIBGRAPI_2019
#ifdef SIBGRAPI_2019
  shader_blend_pass->Bind();
  shader_blend_pass->SetUniformTexture2D("EyeBufferSlice", lfb_object.GetColorAttachmentID(2), 0);
  shader_blend_pass->BindUniform("EyeBufferSlice");

  // render quad to composite with screen
  glBegin(GL_QUADS);
  glVertex3f(-1, -1, 0);
  glVertex3f(+1, -1, 0);
  glVertex3f(+1, +1, 0);
  glVertex3f(-1, +1, 0);
  glEnd();

  gl::Shader::Unbind();
#else
  m_rdr_frame_to_screen.DrawHigherResolutionWithDownScale(
    lfb_object.GetColorAttachmentID(2),
    shader_width,
    shader_height);
#endif
}

void SBTMDirectionalOcclusionShading::UpScalingRedraw ()
{
  GLuint occlusion_buffer_next = 0;
  GLuint occlusion_buffer_prev = 1;

  GLuint eye_buffer = 2;

  lfb_object.Bind();

  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT | GL_VIEWPORT_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glDisable(GL_BLEND);
  glViewport(0, 0, shader_width, shader_height);
  GLuint attachmentsi[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(2, attachmentsi);

  ps_shader_rendering->Bind();

  // https://stackoverflow.com/questions/44756898/opengl-different-clear-color-for-individual-color-attachments
  // Using opengl 4.4+
  glClearTexImage(lfb_object.GetColorAttachmentID(2), 0, GL_RGBA, GL_FLOAT, 0);
  // We don't need to update the other occlusion buffer since the blending is not applied here
  const float init_opacity[4] = { 1, 0, 0, 0 };
  glClearTexImage(lfb_object.GetColorAttachmentID(1), 0, GL_RGBA, GL_FLOAT, &init_opacity);
  glClearTexImage(lfb_object.GetColorAttachmentID(0), 0, GL_RGBA, GL_FLOAT, &init_opacity);

  ps_shader_rendering->SetUniform("CameraDirection", cam_dir);
  ps_shader_rendering->BindUniform("CameraDirection");

  quad_vao->Bind();
  quad_vbo->Bind();
  quad_ibo->Bind();

  // Render each slice using glDrawElements
  number_of_passes = 0;
  for (int i = 0; i < proxy_geom.quads.size() && number_of_passes < max_number_of_passes; i++)
  {
    lfb_object.SetRenderTargetColoAttachment(0, eye_buffer);
    lfb_object.SetRenderTargetColoAttachment(1, occlusion_buffer_next);
    // TODO: change the swap buffer strategy
    // TODO: https://gist.github.com/roxlu/6b0d2081675c24c607d0

    ps_shader_rendering->SetUniformTexture2D("EyeBufferPrev", lfb_object.GetColorAttachmentID(eye_buffer), 2);
    ps_shader_rendering->BindUniform("EyeBufferPrev");

    ps_shader_rendering->SetUniformTexture2D("OcclusionBufferPrev", lfb_object.GetColorAttachmentID(occlusion_buffer_prev), 3);
    ps_shader_rendering->BindUniform("OcclusionBufferPrev");

    ps_shader_rendering->SetUniform("DistanceFromNearSlice", proxy_geom.quads[i].pos_from_near);
    ps_shader_rendering->BindUniform("DistanceFromNearSlice");

    quad_vao->DrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT);

    occlusion_buffer_prev = occlusion_buffer_next;
    occlusion_buffer_next = (occlusion_buffer_next + 1) % 2;

    number_of_passes++;
  }

  gl::BufferObject::Unbind(GL_ARRAY_BUFFER);
  gl::BufferObject::Unbind(GL_ELEMENT_ARRAY_BUFFER);
  gl::ArrayObject::Unbind();

  ps_shader_rendering->Unbind();

  glPopAttrib();

  lfb_object.Unbind();
  // BLEND THE RESULT WITH THE BACKGROUND COLOR
  // 27. CompositWithWindowFrameBuffer(eye_buffer)
//#define SIBGRAPI_2019
#ifdef SIBGRAPI_2019
  shader_blend_pass->Bind();
  shader_blend_pass->SetUniformTexture2D("EyeBufferSlice", lfb_object.GetColorAttachmentID(2), 0);
  shader_blend_pass->BindUniform("EyeBufferSlice");

  // render quad to composite with screen
  glBegin(GL_QUADS);
  glVertex3f(-1, -1, 0);
  glVertex3f(+1, -1, 0);
  glVertex3f(+1, +1, 0);
  glVertex3f(-1, +1, 0);
  glEnd();

  gl::Shader::Unbind();
#else
  m_rdr_frame_to_screen.DrawLowerResolutionWithUpScale(
    lfb_object.GetColorAttachmentID(2),
    shader_width,
    shader_height);
#endif
}

void SBTMDirectionalOcclusionShading::Reshape(int w, int h)
{
  BaseVolumeRenderer::Reshape(w, h);

  shader_width  = m_rdr_frame_to_screen.GetScreenOutputTexture()->GetWidth();
  shader_height = m_rdr_frame_to_screen.GetScreenOutputTexture()->GetHeight();

  lfb_object.Resize(shader_width, shader_height);

  ps_shader_rendering->Bind();
  ps_shader_rendering->SetUniform("ScreenSize", glm::vec2(shader_width, shader_height));
  ps_shader_rendering->BindUniform("ScreenSize");

  shader_blend_pass->Bind();
  shader_blend_pass->SetUniform("ScreenSize", glm::vec2(shader_width, shader_height));
  shader_blend_pass->BindUniform("ScreenSize");

  gl::PipelineShader::Unbind();
}

void SBTMDirectionalOcclusionShading::SetImGuiComponents ()
{
  ImGui::Separator();
  ImGui::Text("- Step Size: ");
  if (ImGui::DragFloat("###RayCasting1PassUIIntegrationStepSize", &m_u_step_size, 0.01f, 0.01f, 100.0f, "%.2f"))
    SetOutdated();
  ImGui::Separator();

  if (AddImGuiMultiSampleOptions())
  {
    shader_width  = m_rdr_frame_to_screen.GetScreenOutputTexture()->GetWidth();
    shader_height = m_rdr_frame_to_screen.GetScreenOutputTexture()->GetHeight();

    lfb_object.Resize(shader_width, shader_height);

    ps_shader_rendering->Bind();
    ps_shader_rendering->SetUniform("ScreenSize", glm::vec2(shader_width, shader_height));
    ps_shader_rendering->BindUniform("ScreenSize");

    shader_blend_pass->Bind();
    shader_blend_pass->SetUniform("ScreenSize", glm::vec2(shader_width, shader_height));
    shader_blend_pass->BindUniform("ScreenSize");

    gl::PipelineShader::Unbind();
  }

  ImGui::Text("Cone Aperture Angle");
  if (ImGui::DragFloat("###ConeAperture", &cone_half_angle, 0.5f, 0.5f, 89.5f))
  {
    SetOutdated();
  }

  ImGui::Text("Grid Size: %dx%d", (int)grid_size.x, (int)grid_size.y);
  ImGui::SameLine();
  if (ImGui::Button("-###DecreaseGridSize"))
  {
    grid_size = grid_size - 1.0f;
    SetOutdated();
  }
  ImGui::SameLine();
  if (ImGui::Button("+###IncraseGridSize"))
  {
    grid_size = grid_size + 1.0f;
    SetOutdated();
  }

  ImGui::Text("Attenuation Factor");
  if (ImGui::DragFloat("###OcclusionAttenuationFactor", &occ_ui_weight_percentage, 0.001f, 0.0f, 1.0f))
  {
    SetOutdated();
  }

  ImGui::Text("Number of passes: %d", number_of_passes);
}

void SBTMDirectionalOcclusionShading::CreateSlicePass(vis::StructuredGridVolume* vr_volume)
{
  glm::vec3 vol_resolution = glm::vec3(m_ext_data_manager->GetCurrentStructuredVolume()->GetWidth(),
    m_ext_data_manager->GetCurrentStructuredVolume()->GetHeight(),
    m_ext_data_manager->GetCurrentStructuredVolume()->GetDepth());

  glm::vec3 vol_voxelsize = glm::vec3(m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleX(),
    m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleY(),
    m_ext_data_manager->GetCurrentStructuredVolume()->GetScaleZ());

  glm::vec3 vol_aabb = vol_resolution * vol_voxelsize;

  ps_shader_rendering = new gl::PipelineShader();
  ps_shader_rendering->AddShaderFile(gl::PipelineShader::TYPE::VERTEX, CPPVOLREND_DIR"structured/sbtmdos/evaluationslice.vert");
  ps_shader_rendering->AddShaderFile(gl::PipelineShader::TYPE::FRAGMENT, CPPVOLREND_DIR"structured/sbtmdos/evaluationslice.frag");
  ps_shader_rendering->LoadAndLink();
    
  ps_shader_rendering->Bind();
  
  if (m_ext_data_manager->GetCurrentVolumeTexture()) ps_shader_rendering->SetUniformTexture3D("TexVolume", m_ext_data_manager->GetCurrentVolumeTexture()->GetTextureID(), 0);
  if (m_glsl_transfer_function) ps_shader_rendering->SetUniformTexture1D("TexTransferFunc", m_glsl_transfer_function->GetTextureID(), 1);
  
  vol_scale = glm::vec3(vr_volume->GetScaleX(), vr_volume->GetScaleY(), vr_volume->GetScaleZ());
  
  ps_shader_rendering->SetUniform("VolumeScaledSizes", vol_aabb);
  ps_shader_rendering->SetUniform("VolumeScales", vol_voxelsize);
  
  ps_shader_rendering->BindUniforms();
  
  gl::ExitOnGLError("Could not get shader uniform locations");
  gl::PipelineShader::Unbind();
}

void SBTMDirectionalOcclusionShading::CreateBlendPass ()
{
  shader_blend_pass = new gl::PipelineShader();
  shader_blend_pass->AddShaderFile(gl::PipelineShader::TYPE::VERTEX, CPPVOLREND_DIR"structured/sbtmdos/blend.vert");
  shader_blend_pass->AddShaderFile(gl::PipelineShader::TYPE::FRAGMENT, CPPVOLREND_DIR"structured/sbtmdos/blend.frag");
  shader_blend_pass->LoadAndLink();

  shader_blend_pass->Bind();
  shader_blend_pass->SetUniform("ScreenSize", glm::vec2(shader_width, shader_height));

  shader_blend_pass->BindUniforms();

  gl::ExitOnGLError("Could not get shader uniform locations");
  gl::PipelineShader::Unbind();

}

void SBTMDirectionalOcclusionShading::ComputeProxyGeometry()
{
  proxy_geom.quads.clear();

  float vol_diag_half = volume_diagonal * 0.5f;
  float Lw = (float)m_ext_data_manager->GetCurrentStructuredVolume()->GetWidth();
  float Lw2 = Lw * 0.5f;
  float Lh = (float)m_ext_data_manager->GetCurrentStructuredVolume()->GetHeight();
  float Lh2 = Lh * 0.5f;
  float Ld = (float)m_ext_data_manager->GetCurrentStructuredVolume()->GetDepth();
  float Ld2 = Ld * 0.5f;

  float s = minimum_z;
  while (s < maximum_z)
  {
    float d = glm::min(m_u_step_size, maximum_z - s);
    glm::vec3 center = cam_eye + cam_dir * (s + d * 0.5f);

    SliceQuad sq;

    sq.pt_lb = center - cam_right * vol_diag_half - cam_up * vol_diag_half;
    sq.tx_lb = glm::vec3((sq.pt_lb.x + Lw2) / Lw, (sq.pt_lb.y + Lh2) / Lh, (sq.pt_lb.z + Ld2) / Ld);

    sq.pt_rb = center + cam_right * vol_diag_half - cam_up * vol_diag_half;
    sq.tx_rb = glm::vec3((sq.pt_rb.x + Lw2) / Lw, (sq.pt_rb.y + Lh2) / Lh, (sq.pt_rb.z + Ld2) / Ld);

    sq.pt_rt = center + cam_right * vol_diag_half + cam_up * vol_diag_half;
    sq.tx_rt = glm::vec3((sq.pt_rt.x + Lw2) / Lw, (sq.pt_rt.y + Lh2) / Lh, (sq.pt_rt.z + Ld2) / Ld);

    sq.pt_lt = center - cam_right * vol_diag_half + cam_up * vol_diag_half;
    sq.tx_lt = glm::vec3((sq.pt_lt.x + Lw2) / Lw, (sq.pt_lt.y + Lh2) / Lh, (sq.pt_lt.z + Ld2) / Ld);

    sq.pos_from_near = (s + d * 0.5f) - minimum_z;

    proxy_geom.quads.push_back(sq);

    s = s + d;
  }

  if (quad_vao != nullptr)
  {
    delete quad_vao;
    delete quad_vbo;
    delete quad_ibo;
  }

  SliceQuad qd = proxy_geom.quads[0];
  // Quad Screen
  const vis::Vertex1p VERTICES[4] = {
    { { qd.pt_lb.x, qd.pt_lb.y, qd.pt_lb.z } },
    { { qd.pt_rb.x, qd.pt_rb.y, qd.pt_rb.z } },
    { { qd.pt_rt.x, qd.pt_rt.y, qd.pt_rt.z } },
    { { qd.pt_lt.x, qd.pt_lt.y, qd.pt_lt.z } } };
  const GLuint INDICES[6] = { 0, 1, 2, 0, 2, 3 };

  // VBO, VAO, IBO
  quad_vao = new gl::ArrayObject(1);
  quad_vao->Bind();

  quad_vbo = new gl::BufferObject(GL_ARRAY_BUFFER);
  quad_ibo = new gl::BufferObject(GL_ELEMENT_ARRAY_BUFFER);

  // bind the VBO to the VAO
  quad_vbo->SetBufferData(sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
  quad_vao->SetVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);

  // bind the IBO to the VAO
  quad_ibo->SetBufferData(sizeof(INDICES), INDICES, GL_STATIC_DRAW);

  gl::BufferObject::Unbind(GL_ARRAY_BUFFER);
  gl::BufferObject::Unbind(GL_ELEMENT_ARRAY_BUFFER);
  gl::ArrayObject::Unbind();
}

void SBTMDirectionalOcclusionShading::ComputeMinMaxZ(glm::vec3 vert)
{
  glm::mat4 vm = ViewMatrix;

  glm::vec4 pt = vm * glm::vec4(vert, 1);

  minimum_z = glm::min(minimum_z, -pt.z);
  maximum_z = glm::max(maximum_z, -pt.z);
}

// 8 vertices
void SBTMDirectionalOcclusionShading::ComputeMinMaxZBoudingBox(glm::vec3 aabb)
{
  minimum_z = +99999;
  maximum_z = -99999;

  // v1 -> +x +y +z
  ComputeMinMaxZ(glm::vec3(+aabb.x, +aabb.y, +aabb.z));
  // v2 -> +x -y +z
  ComputeMinMaxZ(glm::vec3(+aabb.x, -aabb.y, +aabb.z));
  // v3 -> -x +y +z
  ComputeMinMaxZ(glm::vec3(-aabb.x, +aabb.y, +aabb.z));
  // v4 -> -x -y +z
  ComputeMinMaxZ(glm::vec3(-aabb.x, -aabb.y, +aabb.z));
  // v5 -> +x +y -z
  ComputeMinMaxZ(glm::vec3(+aabb.x, +aabb.y, -aabb.z));
  // v6 -> +x -y -z
  ComputeMinMaxZ(glm::vec3(+aabb.x, -aabb.y, -aabb.z));
  // v7 -> -x +y -z
  ComputeMinMaxZ(glm::vec3(-aabb.x, +aabb.y, -aabb.z));
  // v8 -> -x -y -z
  ComputeMinMaxZ(glm::vec3(-aabb.x, -aabb.y, -aabb.z));
}