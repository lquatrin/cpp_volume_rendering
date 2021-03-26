#ifndef VOL_VIS_UTILS_LIGHT_SOURCE_LIST_H
#define VOL_VIS_UTILS_LIGHT_SOURCE_LIST_H

#include <volvis_utils/structuredgridvolume.h>
#include <volvis_utils/transferfunction.h>

#include <iostream>

namespace vis
{
  enum LIGHT_SOURCE_TYPE : unsigned int {
    POINT_LIGHT = 0,
    DIRECTIONAL_LIGHT = 1,
    SPOIT_LIGHT = 2,
  };

  class LightSourceData
  {
  public:
    LightSourceData ();
    LightSourceData (glm::vec3 _pos,
                     glm::vec3 _fwd,
                     glm::vec3 _up,
                     glm::vec3 _rgt,
                     float _spotangle);
    ~LightSourceData ();

    std::string name;

    glm::vec3 color;
    glm::vec3 specular;

    glm::vec3 position;

    glm::vec3 z_axis;
    glm::vec3 y_axis;
    glm::vec3 x_axis;

    float spot_light_angle;
    float spot_light_angle_rad;

    float energy_density;

    LIGHT_SOURCE_TYPE l_type;

  protected:

  private:

  };

  class LightSourceListItem
  {
  public:
    LightSourceListItem ();
    ~LightSourceListItem ();

    std::vector<vis::LightSourceData> m_lightsources;
    std::string l_name;
  };

  class LightSourceList
  {
  public:
    LightSourceList ();
    ~LightSourceList ();

    bool ReadLightSourceLists (std::string filepath);

    int NumberOfLists ();
    vis::LightSourceListItem* GetList (int i);

  protected:
    std::vector<vis::LightSourceListItem> m_vec_lsource_lists;

  private:

  };
}

#endif