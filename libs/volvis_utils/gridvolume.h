/**
 * Base Grid Volume Class.
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef VOL_VIS_UTILS_GRID_VOLUME_H
#define VOL_VIS_UTILS_GRID_VOLUME_H

#include <iostream>
#include <string>

#include <glm/glm.hpp>

namespace vis
{
  enum class GRID_VOLUME_DATA_TYPE : unsigned int {
    STRUCTURED = 0,
    UNSTRUCTURED = 1,
    NONE_DATA_TYPE = 2
  };

  class GridVolume
  {
  public:
    GridVolume (std::string name = "Unknown");
    ~GridVolume ();
  
    std::string GetName ();
    void SetName (std::string name);
  
    double GetDiagonal ();
    virtual glm::dvec3 GetGridCenterPoint () = 0;
    virtual glm::dvec3 GetGridBBoxMin () = 0;
    virtual glm::dvec3 GetGridBBoxMax () = 0;

  protected:
    virtual void DestroyData ();
  
  private: 
    std::string m_vol_name;
  };
}

#endif