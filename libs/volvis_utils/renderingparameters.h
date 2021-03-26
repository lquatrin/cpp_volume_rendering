/**
 * renderingparameters.h
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef VIS_UTILS_RENDERING_PARAMETERS_VOLUME_RENDERING_H
#define VIS_UTILS_RENDERING_PARAMETERS_VOLUME_RENDERING_H

#include <glm/glm.hpp>

#include <cstring>
#include <sstream>

#include <gl_utils/texture1d.h>
#include <gl_utils/texture2d.h>
#include <gl_utils/texture3d.h>

#include <vis_utils/camera.h>

#include <volvis_utils/lightsourcelist.h>

#define USING_IMGUI

namespace vis
{
  class RenderingParameters
  {
  public:
    RenderingParameters();
    ~RenderingParameters();

    void RequestSaveScreenshot ();
    bool TakeScreenshot ();

    void SetDefaultScreenshotName (std::string ndef);
    std::string GetDefaultScreenshotName ();

    ///////////////////////////////////////
    // Blinn Phong Data
    void SetPhongParameters (float amb, 
                             float diff, 
                             float spec, 
                             float shini);
    float GetBlinnPhongKambient ();
    float GetBlinnPhongKdiffuse ();
    float GetBlinnPhongKspecular ();
    float GetBlinnPhongNshininess ();

    ///////////////////////////////////////
    // Light Source Data
    int GetNumberOfLightSources ();
    void CreateNewLightSource ();
    void CreateNewLightSource (vis::LightSourceData lsd);
    void EraseLightSource (int i);
    void EraseAllLightSources ();

    vis::LightSourceData* GetLightSourceData (int i);

    glm::vec3 GetLightSourceColor ();

    glm::vec3 GetLightSourceSpecular ();

    void SetBlinnPhongLightingPosition (glm::vec3 lightpos);
    glm::vec3 GetBlinnPhongLightingPosition ();

    void SetBlinnPhongLightSourceCameraVectors (glm::vec3 lcamforward,
                                                glm::vec3 lcamup, 
                                                glm::vec3 lcamright);
    glm::vec3 GetBlinnPhongLightSourceCameraForward ();
    glm::vec3 GetBlinnPhongLightSourceCameraUp ();
    glm::vec3 GetBlinnPhongLightSourceCameraRight ();

    float GetSpotLightMaxAngle ();

    ///////////////////////////////////////
    // Screen properties
    void SetScreenSize (int width, int height);
    int GetScreenWidth ();
    int GetScreenHeight ();
    
    ///////////////////////////////////////
    // Screen properties
    vis::Camera* GetCamera ();

  protected:
  
  private:
    vis::Camera s_camera;

    unsigned int screen_width;
    unsigned int screen_height;

    float m_blinnphong_ka;
    float m_blinnphong_kd;
    float m_blinnphong_ks;
    float m_blinnphong_shininess;

    int m_current_light_source_id;

    std::vector<vis::LightSourceData> m_vec_light_sources;

    bool requestsavescreenshot;
    std::string default_screenshot_name;
  };

}

#endif

/*
  public:
    RenderingParameters();
    ~RenderingParameters();

    float GetBlinnPhongKa();
    float GetBlinnPhongKd();
    float GetBlinnPhongKs();
    float GetBlinnPhongShininess();

    void CreateNewLightSource();
    int GetNumberOfLightSources();
    LightSource* GetLightSource(int ith);
    void EraseLightSource(int i);

  protected:

  private:

  };
}

#endif
*/