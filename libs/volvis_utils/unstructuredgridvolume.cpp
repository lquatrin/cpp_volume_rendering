#include "unstructuredgridvolume.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

namespace vis
{
  /////////////////////
  // Public Methods  //
  /////////////////////
  UnstructuredGridVolume::UnstructuredGridVolume (std::string vname)
    : GridVolume(vname)
  {}
  
  UnstructuredGridVolume::~UnstructuredGridVolume ()
  {
    DestroyData();
  }
  
  glm::dvec3 UnstructuredGridVolume::GetGridCenterPoint ()
  {
    return glm::dvec3(0);
  }
  glm::dvec3 UnstructuredGridVolume::GetGridBBoxMin ()
  {
    return glm::dvec3(0);
  }
  glm::dvec3 UnstructuredGridVolume::GetGridBBoxMax ()
  {
    return glm::dvec3(0);
  }
 
  /////////////////////
  // Private Methods //
  /////////////////////
  void UnstructuredGridVolume::DestroyData ()
  {
  }
}