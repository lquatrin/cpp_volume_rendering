#include "volrenderbase.h"

#include "defines.h"

BaseVolumeRenderer::BaseVolumeRenderer()
  : vr_pixel_multiscaling_support(false)
  , vr_pixel_multiscaling_mode(0)
  , m_rdr_frame_to_screen(CPPVOLREND_DIR"../../libs/vis_utils/shader/")
{
  SetBuilt(false);

  SetOutdated();
}

BaseVolumeRenderer::~BaseVolumeRenderer ()
{
  Clean();
}

void BaseVolumeRenderer::SetExternalResources (vis::DataManager* data_mgr, vis::RenderingParameters* rdr_prm)
{
  m_ext_data_manager = data_mgr;
  m_ext_rendering_parameters = rdr_prm;
}

void BaseVolumeRenderer::Clean ()
{
  m_rdr_frame_to_screen.Clean();
  SetBuilt(false);
}

void BaseVolumeRenderer::ReloadShaders () {}

void BaseVolumeRenderer::Redraw () {}

void BaseVolumeRenderer::MultiSampleRedraw () {}

void BaseVolumeRenderer::DownScalingRedraw () {}

void BaseVolumeRenderer::UpScalingRedraw () {}

void BaseVolumeRenderer::Reshape (int w, int h)
{
  if (IsPixelMultiScalingSupported() && GetCurrentMultiScalingMode() > 0)
  {
    if (GetCurrentMultiScalingMode() == MULTIPLE_RAYS_PER_PIXEL)
    {
      m_rdr_frame_to_screen.UpdateScreenResolutionMultiScaling(w, h,
        MULTISAMPLE_NUMBEROFSAMPLES_W, MULTISAMPLE_NUMBEROFSAMPLES_H);
    }
    else if (GetCurrentMultiScalingMode() == DOWN_SCALING_RENDER)
    {
      m_rdr_frame_to_screen.UpdateScreenResolutionMultiScaling(w, h,
        MULTISAMPLE_NUMBEROFSAMPLES_W, MULTISAMPLE_NUMBEROFSAMPLES_H);
    }
    else if (GetCurrentMultiScalingMode() == UP_SCALING_RENDER)
    {
      m_rdr_frame_to_screen.UpdateScreenResolutionMultiScaling(w, h,
        -MULTISAMPLE_NUMBEROFSAMPLES_W, -MULTISAMPLE_NUMBEROFSAMPLES_H);
    }
  }
  else
  {
    m_rdr_frame_to_screen.UpdateScreenResolution(w, h);
  }
  gl::ExitOnGLError("Error on Reshape (screen_output texture).");
  SetOutdated();
}

void BaseVolumeRenderer::SetImGuiComponents () {}

void BaseVolumeRenderer::PrepareRender (vis::Camera* camera)
{
  if (IsOutdated())
  {
    Update(camera);
    vr_outdated = false;
  }
}

void BaseVolumeRenderer::SetOutdated ()
{
  vr_outdated = true;
}

bool BaseVolumeRenderer::IsOutdated ()
{
  return vr_outdated;
}

bool BaseVolumeRenderer::IsBuilt ()
{
  return vr_built;
}

bool BaseVolumeRenderer::IsPixelMultiScalingSupported ()
{
  return vr_pixel_multiscaling_support;
}

int BaseVolumeRenderer::GetCurrentMultiScalingMode ()
{
  return IsPixelMultiScalingSupported() ? vr_pixel_multiscaling_mode : 0;
}

void BaseVolumeRenderer::SetCurrentMultiScalingMode (int f)
{
  vr_pixel_multiscaling_mode = f;
}

void BaseVolumeRenderer::SetBuilt (bool b_built)
{
  vr_built = b_built;
}

bool BaseVolumeRenderer::AddImGuiMultiSampleOptions ()
{
  bool f = false;
  if (IsPixelMultiScalingSupported())
  {
    int e = GetCurrentMultiScalingMode();

    if (ImGui::RadioButton("Single Ray Per Pixel###SRPP", &e, 0))
    {
      SetCurrentMultiScalingMode(0);
      m_rdr_frame_to_screen.UpdateScreenResolution(
        m_ext_rendering_parameters->GetScreenWidth(),
        m_ext_rendering_parameters->GetScreenHeight());
      SetOutdated();
      f = true;
    }
    if (ImGui::RadioButton("Multiple Rays Per Pixel###MRPP", &e, 1))
    {
      SetCurrentMultiScalingMode(1);
      m_rdr_frame_to_screen.SetMultiResolutionScreenMultiplier(
        glm::ivec2(MULTISAMPLE_NUMBEROFSAMPLES_W, MULTISAMPLE_NUMBEROFSAMPLES_H)
      );
      m_rdr_frame_to_screen.UpdateScreenResolutionMultiScaling(
        m_ext_rendering_parameters->GetScreenWidth(),
        m_ext_rendering_parameters->GetScreenHeight());
      SetOutdated();
      f = true;
    }
    if (ImGui::RadioButton("DownScaling Render###DSR", &e, 2))
    {
      SetCurrentMultiScalingMode(2);
      m_rdr_frame_to_screen.SetMultiResolutionScreenMultiplier(
        glm::ivec2(MULTISAMPLE_NUMBEROFSAMPLES_W, MULTISAMPLE_NUMBEROFSAMPLES_H)
      );
      m_rdr_frame_to_screen.UpdateScreenResolutionMultiScaling(
        m_ext_rendering_parameters->GetScreenWidth(),
        m_ext_rendering_parameters->GetScreenHeight());
      SetOutdated();
      f = true;
    }
    if (ImGui::RadioButton("UpScaling Render###USR", &e, 3))
    {
      SetCurrentMultiScalingMode(3);
      m_rdr_frame_to_screen.SetMultiResolutionScreenMultiplier(
        glm::ivec2(-MULTISAMPLE_NUMBEROFSAMPLES_W, -MULTISAMPLE_NUMBEROFSAMPLES_H)
      );
      m_rdr_frame_to_screen.UpdateScreenResolutionMultiScaling(
        m_ext_rendering_parameters->GetScreenWidth(),
        m_ext_rendering_parameters->GetScreenHeight());
      SetOutdated();
      f = true;
    }

    // https://eliasdaler.github.io/using-imgui-with-sfml-pt2/#using-imgui-with-stl
    static auto vector_getter = [](void* vec, int idx, const char** out_text)
    {
      auto& vector = *static_cast<std::vector<std::string>*>(vec);
      if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
      *out_text = vector.at(idx).c_str();
      return true;
    };

    int ik = m_rdr_frame_to_screen.GetImageKernelFilter();
    std::vector<std::string> k_filters;
    k_filters.push_back("Box");
    k_filters.push_back("Hat");
    k_filters.push_back("Catmull Rom");
    k_filters.push_back("Mitchell Netravali");
    k_filters.push_back("CardinalBspline");
    k_filters.push_back("CardinalOMOMS");
    if (ImGui::Combo("###ImArrayLoadImgKernelFilters", &ik, vector_getter,
      static_cast<void*>(&k_filters), k_filters.size()))
    {
      m_rdr_frame_to_screen.SetImageKernelFilter(ik);
      f = true;
    }
  }
  ImGui::Separator();
  return f;
}
