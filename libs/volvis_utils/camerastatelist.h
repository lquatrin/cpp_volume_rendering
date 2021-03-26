#ifndef VOL_VIS_UTILS_CAMERA_STATE_LIST_H
#define VOL_VIS_UTILS_CAMERA_STATE_LIST_H

#include <volvis_utils/structuredgridvolume.h>
#include <volvis_utils/transferfunction.h>

#include <iostream>

#include <vis_utils/camera.h>

namespace vis
{
  class CameraStateList
  {
  public:
    CameraStateList ();
    ~CameraStateList ();

    bool ReadCameraStates (std::string filepath);

    int NumberOfCameraStates ();
    vis::CameraData* GetCameraState (int i);

  protected:
    std::vector<vis::CameraData> m_vec_camera_data;

  private:

  };
}

#endif