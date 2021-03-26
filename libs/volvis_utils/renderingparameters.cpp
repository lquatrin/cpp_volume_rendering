/**
 * renderingparameters.cpp
 *
 * https://docs.unrealengine.com/en-US/Engine/Audio/DistanceModelAttenuation
 * https://en.wikipedia.org/wiki/Attenuation
 * http://learnwebgl.brown37.net/09_lights/lights_attenuation.html
 * https://www.miniphysics.com/light-attenuation.html
 * https://www.sciencedirect.com/topics/earth-and-planetary-sciences/light-attenuation
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#include <volvis_utils/renderingparameters.h>

namespace vis
{
  RenderingParameters::RenderingParameters ()
    : s_camera(200.0f)
  {
    screen_width = 768;
    screen_height = 768;

    m_blinnphong_ka = 0.5f;
    m_blinnphong_kd = 0.5f;
    m_blinnphong_ks = 0.8f;
    m_blinnphong_shininess = 30.0f;

    m_current_light_source_id = 0;

    requestsavescreenshot = false;
    default_screenshot_name = "output.png";
  }

  RenderingParameters::~RenderingParameters ()
  {}

  void RenderingParameters::RequestSaveScreenshot ()
  {
    requestsavescreenshot = true;
  }

  bool RenderingParameters::TakeScreenshot ()
  {
    bool ret = requestsavescreenshot;
    requestsavescreenshot = false;
    return ret;
  }

  void RenderingParameters::SetDefaultScreenshotName (std::string ndef)
  {
    default_screenshot_name = ndef;
  }

  std::string RenderingParameters::GetDefaultScreenshotName ()
  {
    return default_screenshot_name;
  }

  void RenderingParameters::SetPhongParameters (float amb, 
                                                float diff, 
                                                float spec, 
                                                float shini)
  {
    m_blinnphong_ka = amb;
    m_blinnphong_kd = diff;
    m_blinnphong_ks = spec;
    m_blinnphong_shininess = shini;
  }

  float RenderingParameters::GetBlinnPhongKambient()
  {
    return m_blinnphong_ka;
  }

  float RenderingParameters::GetBlinnPhongKdiffuse ()
  {
    return m_blinnphong_kd;
  }

  float RenderingParameters::GetBlinnPhongKspecular ()
  {
    return m_blinnphong_ks;
  }

  float RenderingParameters::GetBlinnPhongNshininess ()
  {
    return m_blinnphong_shininess;
  }

  int RenderingParameters::GetNumberOfLightSources ()
  {
    return m_vec_light_sources.size();
  }

  void RenderingParameters::CreateNewLightSource ()
  {
    m_vec_light_sources.push_back(LightSourceData());
  }

  void RenderingParameters::CreateNewLightSource (vis::LightSourceData lsd)
  {
    m_vec_light_sources.push_back(lsd);
  }

  void RenderingParameters::EraseLightSource (int i)
  {
    if (GetNumberOfLightSources() > 1)
    {
      m_vec_light_sources.erase(m_vec_light_sources.begin() + i);
      m_current_light_source_id = glm::clamp(m_current_light_source_id, 0, GetNumberOfLightSources() - 1);
    }
  }

  void RenderingParameters::EraseAllLightSources ()
  {
    while (GetNumberOfLightSources() > 0)
    {
      m_vec_light_sources.erase(m_vec_light_sources.begin());
    }
  }

  LightSourceData* RenderingParameters::GetLightSourceData (int i)
  {
    return &m_vec_light_sources[i];
  }

  glm::vec3 RenderingParameters::GetLightSourceColor ()
  {
    return m_vec_light_sources[m_current_light_source_id].color;
  }

  glm::vec3 RenderingParameters::GetLightSourceSpecular ()
  {
    return m_vec_light_sources[m_current_light_source_id].specular;
  }

  void RenderingParameters::SetBlinnPhongLightingPosition (glm::vec3 lightpos)
  {
    m_vec_light_sources[m_current_light_source_id].position = lightpos;
  }

  glm::vec3 RenderingParameters::GetBlinnPhongLightingPosition ()
  {
    return m_vec_light_sources[m_current_light_source_id].position;
  }

  void RenderingParameters::SetBlinnPhongLightSourceCameraVectors (glm::vec3 lcamforward, glm::vec3 lcamup, glm::vec3 lcamright)
  {
    m_vec_light_sources[m_current_light_source_id].z_axis = -lcamforward;
    m_vec_light_sources[m_current_light_source_id].y_axis = lcamup;
    m_vec_light_sources[m_current_light_source_id].x_axis = lcamright;
  }

  glm::vec3 RenderingParameters::GetBlinnPhongLightSourceCameraForward ()
  {
    return -m_vec_light_sources[m_current_light_source_id].z_axis;
  }

  glm::vec3 RenderingParameters::GetBlinnPhongLightSourceCameraUp ()
  {
    return m_vec_light_sources[m_current_light_source_id].y_axis;
  }

  glm::vec3 RenderingParameters::GetBlinnPhongLightSourceCameraRight ()
  {
    return m_vec_light_sources[m_current_light_source_id].x_axis;
  }

  float RenderingParameters::GetSpotLightMaxAngle ()
  {
    return m_vec_light_sources[m_current_light_source_id].spot_light_angle;
  }

  void RenderingParameters::SetScreenSize (int width, int height)
  {
    screen_width = width;
    screen_height = height;
  }

  int RenderingParameters::GetScreenWidth ()
  {
    return screen_width;
  }

  int RenderingParameters::GetScreenHeight ()
  {
    return screen_height;
  }

  vis::Camera* RenderingParameters::GetCamera ()
  {
    return &s_camera;
  }
}