/**
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#include "camerastatelist.h"

#include <file_utils/pvm.h>
#include <file_utils/rawloader.h>

#include <fstream>

#include <volvis_utils/transferfunction1d.h>

namespace vis
{
  CameraStateList::CameraStateList ()
  {

  }

  CameraStateList::~CameraStateList ()
  {
    m_vec_camera_data.clear();
  }

  bool CameraStateList::ReadCameraStates (std::string filepath)
  {
    m_vec_camera_data.clear();

    std::ifstream f_opencamerastates(filepath);
    if (!f_opencamerastates.is_open())
    {
      std::cout << "Error: Unable to read camera states." << std::endl;
      exit(EXIT_FAILURE);
    }

    std::cout << "Reading camera states..." << std::endl;
    std::string s_line;
    int ith = 1;
    while (!f_opencamerastates.eof())
    {
      vis::CameraData c_data;

      s_line.clear();
      std::getline(f_opencamerastates, s_line);
      std::cout << ith << ". " << s_line;

      // Set camera setup name
      c_data.cam_setup_name = s_line;

      s_line.clear();
      std::getline(f_opencamerastates, s_line);
      std::cout << " - " << s_line << std::endl;

      if (s_line.compare("FLIGHT") == 0)
      {
        c_data.c_type = vis::Camera::FLIGHT;

      }
      else if (s_line.compare("ARCBALL") == 0)
      {
        c_data.c_type = vis::Camera::ARCBALL;

        glm::vec3 c_pos;
        f_opencamerastates >> c_pos.x >> c_pos.y >> c_pos.z;
        c_data.eye = c_pos;

        glm::vec3 c_center;
        f_opencamerastates >> c_center.x >> c_center.y >> c_center.z;
        c_data.center = c_center;

        glm::vec3 c_up;
        f_opencamerastates >> c_up.x >> c_up.y >> c_up.z;
        c_data.up = c_up;

        s_line.clear();
        std::getline(f_opencamerastates, s_line);
      }

      m_vec_camera_data.push_back(c_data);

      ith++;
    }
    f_opencamerastates.close();

    return m_vec_camera_data.size() > 0;
  }

  int CameraStateList::NumberOfCameraStates ()
  {
    return m_vec_camera_data.size();
  }

  vis::CameraData * CameraStateList::GetCameraState (int i)
  {
    return &m_vec_camera_data[i];
  }
}