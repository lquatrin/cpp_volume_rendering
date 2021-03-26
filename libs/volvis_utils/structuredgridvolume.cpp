#include "structuredgridvolume.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>

namespace vis
{
  /////////////////////
  // Public Methods  //
  /////////////////////
  StructuredGridVolume::StructuredGridVolume (std::string vname, unsigned int width, unsigned int height, unsigned int depth)
    : GridVolume(vname)
    , m_width(width)
    , m_height(height)
    , m_depth(depth)
    , m_scalex(1.0)
    , m_scaley(1.0)
    , m_scalez(1.0)
    , m_grid_center(glm::dvec3(0.0))
    , m_data_storage_size(DataStorageSize::UNKNOWN)
    , m_voxel_values(nullptr)
  {}
  
  StructuredGridVolume::~StructuredGridVolume ()
  {
    DestroyData();
  }
  
  unsigned int StructuredGridVolume::GetWidth ()
  {
    return m_width;
  }
  
  unsigned int StructuredGridVolume::GetHeight ()
  {
    return m_height;
  }
  
  unsigned int StructuredGridVolume::GetDepth ()
  {
    return m_depth;
  }
  
  double StructuredGridVolume::GetScaleX ()
  {
    return m_scalex;
  }
  
  double StructuredGridVolume::GetScaleY ()
  {
    return m_scaley;
  }
  
  double StructuredGridVolume::GetScaleZ ()
  {
    return m_scalez;
  }
  
  glm::dvec3 StructuredGridVolume::GetScale ()
  {
    return glm::dvec3(GetScaleX(), GetScaleY(), GetScaleZ());
  }
  
  void StructuredGridVolume::SetScale (double sx, double sy, double sz)
  {
    m_scalex = sx;
    m_scaley = sy;
    m_scalez = sz;
  }
  
  glm::dvec3 StructuredGridVolume::GetGridCenterPoint ()
  {
    return m_grid_center;
  }
  
  glm::dvec3 StructuredGridVolume::GetGridBBoxMin ()
  {
    glm::dvec3 grid_scaled = glm::dvec3(GetScaleX() * (double)GetWidth(),
                                        GetScaleY() * (double)GetHeight(),
                                        GetScaleZ() * (double)GetDepth());
  
    return m_grid_center - (grid_scaled * 0.5);
  }
  
  glm::dvec3 StructuredGridVolume::GetGridBBoxMax ()
  {
    glm::dvec3 grid_scaled = glm::dvec3(GetScaleX() * (double)GetWidth(),
                                        GetScaleY() * (double)GetHeight(),
                                        GetScaleZ() * (double)GetDepth());
  
    return m_grid_center + (grid_scaled * 0.5);
  }

  double StructuredGridVolume::GetDiagonal ()
  {
    double v_w = GetWidth()  * GetScaleX();
    double v_h = GetHeight() * GetScaleY();
    double v_d = GetDepth()  * GetScaleZ();
    return glm::sqrt(v_w * v_w + v_h * v_h + v_d * v_d);
  }

  
  bool StructuredGridVolume::IsOutOfBoundary (unsigned int x, unsigned int y, unsigned int z)
  {
    return (x < 0 || y < 0 || z < 0 || x >= GetWidth() || y >= GetHeight() || z >= GetDepth());
  }

  void StructuredGridVolume::SetArrayData (void* input_vol_data, DataStorageSize dss)
  {
    m_data_storage_size = dss;
    m_voxel_values = input_vol_data;
  }

  void* StructuredGridVolume::GetArrayData ()
  {
    return m_voxel_values;
  }

  double StructuredGridVolume::GetNormalizedSample (unsigned int x, unsigned int y, unsigned int z)
  {
    if(m_voxel_values == nullptr || m_data_storage_size == DataStorageSize::UNKNOWN) return 0.0;
    
    if(m_data_storage_size == DataStorageSize::_8_BITS)
    {
      unsigned char* array_vls = static_cast<unsigned char*>(m_voxel_values);
      return (double)array_vls[x + (y * GetWidth()) + (z * GetWidth() * GetHeight())] / (256.0 - 1.0);
    }
    else if(m_data_storage_size == DataStorageSize::_16_BITS)
    {
      unsigned short* array_vls = static_cast<unsigned short*>(m_voxel_values);
      return (double)array_vls[x + (y * GetWidth()) + (z * GetWidth() * GetHeight())] / (65536.0 - 1.0);
    }
    else if (m_data_storage_size == DataStorageSize::_NORMALIZED_F)
    {
      float* array_vls = static_cast<float*>(m_voxel_values);
      return (double)array_vls[x + (y * GetWidth()) + (z * GetWidth() * GetHeight())] / (1.0);
    }
    else if (m_data_storage_size == DataStorageSize::_NORMALIZED_D)
    {
      double* array_vls = static_cast<double*>(m_voxel_values);
      return (double)array_vls[x + (y * GetWidth()) + (z * GetWidth() * GetHeight())] / (1.0);
    }
    return 0.0;
  }

  double StructuredGridVolume::GetNormalizedInterpolatedSample (double i_x, double i_y, double i_z)
  {
    glm::dvec3 bbmin = GetGridBBoxMin();
    glm::dvec3 bbmax = GetGridBBoxMax();

    double x = ((i_x - bbmin.x) / (bbmax.x - bbmin.x)) * (double)(m_width  - 1);
    double y = ((i_y - bbmin.y) / (bbmax.y - bbmin.y)) * (double)(m_height - 1);
    double z = ((i_z - bbmin.z) / (bbmax.z - bbmin.z)) * (double)(m_depth  - 1);

    int x0 = (int)x; int x1 = x0 + 1;
    int y0 = (int)y; int y1 = y0 + 1;
    int z0 = (int)z; int z1 = z0 + 1;

    if (x0 == (double)(m_width - 1))
    {
      x1 = (int)x0;
      x0 = x1 - 1;
    }

    if (y0 == (double)(m_height - 1))
    {
      y1 = (int)y0;
      y0 = y1 - 1;
    }

    if (z0 == (double)(m_depth - 1))
    {
      z1 = (int)z0;
      z0 = z1 - 1;
    }
    
    double xd = (x - (double)x0) / ((double)x1 - (double)x0);
    double yd = (y - (double)y0) / ((double)y1 - (double)y0);
    double zd = (z - (double)z0) / ((double)z1 - (double)z0);
    
    // X interpolation
    double c00 = GetNormalizedSample(x0, y0, z0) * (1.0 - xd) + GetNormalizedSample(x1, y0, z0) * xd;
    double c10 = GetNormalizedSample(x0, y1, z0) * (1.0 - xd) + GetNormalizedSample(x1, y1, z0) * xd;
    double c01 = GetNormalizedSample(x0, y0, z1) * (1.0 - xd) + GetNormalizedSample(x1, y0, z1) * xd;
    double c11 = GetNormalizedSample(x0, y1, z1) * (1.0 - xd) + GetNormalizedSample(x1, y1, z1) * xd;
    
    // Y interpolation
    double c0 = c00 * (1.0 - yd) + c10 * yd;
    double c1 = c01 * (1.0 - yd) + c11 * yd;
    
    // Z interpolation
    double c = c0 * (1.0 - zd) + c1 * zd;
    
    return c;
  }

  unsigned long long StructuredGridVolume::CheckSum ()
  {
    unsigned long long csum = 0;
    for (int i = 0; i < m_width * m_height * m_depth; i++)
    {
      if (m_data_storage_size == DataStorageSize::_8_BITS)
      {
        unsigned char* array_vls = static_cast<unsigned char*>(m_voxel_values);
        csum += (unsigned long long)array_vls[i];
      }
      else if (m_data_storage_size == DataStorageSize::_16_BITS)
      {
        unsigned short* array_vls = static_cast<unsigned short*>(m_voxel_values);
        csum += (unsigned long long)array_vls[i];
      }
      else if (m_data_storage_size == DataStorageSize::_NORMALIZED_F)
      {
        float* array_vls = static_cast<float*>(m_voxel_values);
        csum += (unsigned long long)array_vls[i];
      }
      else if (m_data_storage_size == DataStorageSize::_NORMALIZED_D)
      {
        double* array_vls = static_cast<double*>(m_voxel_values);
        csum += (unsigned long long)array_vls[i];
      }
    }
    return csum;
  }

  double StructuredGridVolume::GetMaxDensity ()
  {
    if (m_data_storage_size == DataStorageSize::_8_BITS)
    {
      return (double)(256.0 - 1.0);
    }
    else if (m_data_storage_size == DataStorageSize::_16_BITS)
    {
      return (double)(65536.0 - 1.0);
    }
    else if (m_data_storage_size == DataStorageSize::_NORMALIZED_F)
    {
      return 1.0;
    }
    else if (m_data_storage_size == DataStorageSize::_NORMALIZED_D)
    {
      return 1.0;
    }
    return 0.0;
  }
  
  /////////////////////
  // Private Methods //
  /////////////////////
  void StructuredGridVolume::DestroyData ()
  {
    if (m_data_storage_size == DataStorageSize::_8_BITS)
    {
      unsigned char* array_vls = static_cast<unsigned char*>(m_voxel_values);
      if(array_vls) delete[] array_vls;
      m_voxel_values = nullptr;
    }
    else if (m_data_storage_size == DataStorageSize::_16_BITS)
    {
      unsigned short* array_vls = static_cast<unsigned short*>(m_voxel_values);
      if (array_vls) delete[] array_vls;
      m_voxel_values = nullptr;
    }
    else if (m_data_storage_size == DataStorageSize::_NORMALIZED_F)
    {
      float* array_vls = static_cast<float*>(m_voxel_values);
      if (array_vls) delete[] array_vls;
      m_voxel_values = nullptr;
    }
    else if (m_data_storage_size == DataStorageSize::_NORMALIZED_D)
    {
      double* array_vls = static_cast<double*>(m_voxel_values);
      if (array_vls) delete[] array_vls;
      m_voxel_values = nullptr;
    }
  }
}