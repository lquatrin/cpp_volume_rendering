/**
 * Class defining an unstructured grid.
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef VOL_VIS_UTILS_UNSTRUCTURED_GRID_VOLUME_H
#define VOL_VIS_UTILS_UNSTRUCTURED_GRID_VOLUME_H

#include <volvis_utils/gridvolume.h>
#include <iostream>
#include <string>

#include <glm/glm.hpp>

namespace vis
{
  class UnstructuredGridVolume : public GridVolume
  {
  public:
    UnstructuredGridVolume (std::string name = "Unknown");
    ~UnstructuredGridVolume ();
  
    virtual glm::dvec3 GetGridCenterPoint ();
    virtual glm::dvec3 GetGridBBoxMin ();
    virtual glm::dvec3 GetGridBBoxMax ();

  protected:
    virtual void DestroyData ();
  
  private: 
  };
}

#endif