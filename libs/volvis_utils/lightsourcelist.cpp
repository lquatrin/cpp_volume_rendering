/**
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#include "lightsourcelist.h"

#include <file_utils/pvm.h>
#include <file_utils/rawloader.h>

#include <fstream>

#include <volvis_utils/transferfunction1d.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

namespace vis
{
  LightSourceData::LightSourceData ()
  {
    color = glm::vec3(1.0f);
    specular = glm::vec3(1.0f);

    position = glm::vec3(0.0f);

    x_axis = glm::vec3(1.0f, 0.0f, 0.0f);
    y_axis = glm::vec3(0.0f, 1.0f, 0.0f);
    z_axis = glm::vec3(0.0f, 0.0f, 1.0f);

    spot_light_angle = (4.0f);

    energy_density = 1.0f;

    l_type = LIGHT_SOURCE_TYPE::DIRECTIONAL_LIGHT;
  }

  LightSourceData::LightSourceData (glm::vec3 _pos,
                                    glm::vec3 _fwd,
                                    glm::vec3 _up,
                                    glm::vec3 _rgt,
                                    float _spotangle)
  {
    position = _pos;
    color = glm::vec3(1.0f);

    z_axis = -_fwd;
    y_axis = _up;
    x_axis = _rgt;

    spot_light_angle = _spotangle;
  
    energy_density = 1.0f;

    l_type = LIGHT_SOURCE_TYPE::DIRECTIONAL_LIGHT;
  }

  LightSourceData::~LightSourceData()
  {
  }


  LightSourceListItem::LightSourceListItem ()
  {
  }

  LightSourceListItem::~LightSourceListItem ()
  {
  }
   
  LightSourceList::LightSourceList ()
  {
  }

  LightSourceList::~LightSourceList ()
  {
    m_vec_lsource_lists.clear();
  }

  bool LightSourceList::ReadLightSourceLists (std::string filepath)
  {
    m_vec_lsource_lists.clear();
  
    std::ifstream f_openlightsources(filepath);
    if (!f_openlightsources.is_open())
    {
      std::cout << "Error: Unable to read light source list." << std::endl;
      exit(EXIT_FAILURE);
    }
  
    std::cout << "Reading light source lists states..." << std::endl;
    std::string s_line;
    int ith = 1;
    while (!f_openlightsources.eof())
    {
      vis::LightSourceListItem l_list;
  
      s_line.clear();
      std::getline(f_openlightsources, s_line);
      std::cout << s_line << std::endl;
  
      // Set camera setup name
      l_list.l_name = s_line;
  
      int number_of_lights;
      f_openlightsources >> number_of_lights;
      for (int i = 0; i < number_of_lights; i++)
      {
        vis::LightSourceData l_data;

        // position
        glm::vec3 c_light;
        f_openlightsources >> c_light.x >> c_light.y >> c_light.z;
        l_data.position = c_light;

        // forward
        glm::vec3 c_forward;
        f_openlightsources >> c_forward.x >> c_forward.y >> c_forward.z;
        l_data.z_axis= -c_forward;
        
        // up
        glm::vec3 c_up;
        f_openlightsources >> c_up.x >> c_up.y >> c_up.z;
        l_data.y_axis = c_up;

        // right
        glm::vec3 c_right;
        f_openlightsources >> c_right.x >> c_right.y >> c_right.z;
        l_data.x_axis = c_right;

        // spotangle
        float spotangle;
        f_openlightsources >> spotangle;
        l_data.spot_light_angle = spotangle;
        l_data.spot_light_angle_rad = (spotangle * glm::pi<float>()) / 180.0f;

        s_line.clear();
        std::getline(f_openlightsources, s_line);

        l_list.m_lightsources.push_back(l_data);
      }
      m_vec_lsource_lists.push_back(l_list);
    }
    f_openlightsources.close();
  
    return m_vec_lsource_lists.size() > 0;
  }

  int LightSourceList::NumberOfLists ()
  {
    return m_vec_lsource_lists.size();
  }

  vis::LightSourceListItem* LightSourceList::GetList (int i)
  {
    return &m_vec_lsource_lists[i];
  }
}