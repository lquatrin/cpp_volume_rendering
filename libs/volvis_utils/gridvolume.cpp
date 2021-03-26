#include "gridvolume.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

namespace vis
{
  /////////////////////
  // Public Methods  //
  /////////////////////
  GridVolume::GridVolume (std::string vname)
    : m_vol_name(vname)
  {}
  
  GridVolume::~GridVolume ()
  {
    DestroyData();
  }
  
  std::string GridVolume::GetName ()
  {
    return m_vol_name;
  }
  
  void GridVolume::SetName (std::string name)
  {
    m_vol_name = name;
  }
 
  double GridVolume::GetDiagonal ()
  {
    return glm::distance(GetGridBBoxMin(), GetGridBBoxMax());
  }
  
  /////////////////////
  // Private Methods //
  /////////////////////
  void GridVolume::DestroyData ()
  {
  }
}