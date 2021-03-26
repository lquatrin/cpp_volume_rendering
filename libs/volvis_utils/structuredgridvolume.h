/**
 * Class defining a structured grid composed by same size voxels.
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef VOL_VIS_UTILS_STRUCTURED_GRID_VOLUME_H
#define VOL_VIS_UTILS_STRUCTURED_GRID_VOLUME_H

#include <volvis_utils/gridvolume.h>
#include <iostream>
#include <string>

#include <glm/glm.hpp>

namespace vis
{
  enum DataStorageSize : unsigned int
  {
    UNKNOWN       = 0, // null data
    _8_BITS       = 1, // unsigned char  [0 -   255]
    _16_BITS      = 2, // unsigned short [0 - 65535]
    _NORMALIZED_F = 3, // float [0.0f - 1.0f]
    _NORMALIZED_D = 4, // double [0.0 - 1.0]
  };

  static DataStorageSize GetStorageSizeType (size_t bytesize)
  {
    if (bytesize == sizeof(unsigned char))
      return DataStorageSize::_8_BITS;
    else if (bytesize == sizeof(unsigned short))
      return DataStorageSize::_16_BITS;
    else if (bytesize == sizeof(float))
      return DataStorageSize::_NORMALIZED_F;
    else if (bytesize == sizeof(double))
      return DataStorageSize::_NORMALIZED_D;
    return DataStorageSize::UNKNOWN;
  }
  
  class StructuredGridVolume : public GridVolume
  {
  public:
    StructuredGridVolume (std::string    name = "Unknown",
                          unsigned int width  = 0,
                          unsigned int height = 0,
                          unsigned int depth  = 0);
    ~StructuredGridVolume ();
  
    unsigned int GetWidth ();
    unsigned int GetHeight ();
    unsigned int GetDepth ();
  
    double GetScaleX ();
    double GetScaleY ();
    double GetScaleZ ();
    glm::dvec3 GetScale ();
    void SetScale (double sx, double sy, double sz);
  
    virtual glm::dvec3 GetGridCenterPoint ();
    virtual glm::dvec3 GetGridBBoxMin ();
    virtual glm::dvec3 GetGridBBoxMax ();
    double GetDiagonal ();

    bool IsOutOfBoundary (unsigned int x, unsigned int y, unsigned int z);
  
    void SetArrayData (void* input_vol_data, DataStorageSize dss);
    void* GetArrayData ();

    double GetNormalizedSample (unsigned int x, unsigned int y, unsigned int z);
    double GetNormalizedInterpolatedSample (double x, double y, double z);

    unsigned long long CheckSum ();

    double GetMaxDensity ();

  protected:
    virtual void DestroyData ();
  
  private: 
  
    unsigned int m_width,  m_height, m_depth;
    double       m_scalex, m_scaley, m_scalez;
    
    glm::dvec3 m_grid_center;
  
    DataStorageSize m_data_storage_size;
    void* m_voxel_values;
  };
}

#endif