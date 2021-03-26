/**
 * Class that computes the Extinction Coefficient Volume using GLSL compute shader.
 *
 * Author: Leonardo Quatrin Campagnolo
 * campagnolo.lq@gmail.com
 *
 * http://blog.ivank.net/fastest-gaussian-blur.html
 **/

#ifndef EXTINCTION_COEFFICIENT_VOLUME_GENERATOR
#define EXTINCTION_COEFFICIENT_VOLUME_GENERATOR

#include <volvis_utils/structuredgridvolume.h>
#include <volvis_utils/transferfunction.h>

#include <gl_utils/texture3d.h>

#include <vector>
#include <iostream>
#include <string>

class ExtinctionCoefficientVolume
{
public:
  ExtinctionCoefficientVolume ();
  ~ExtinctionCoefficientVolume ();

  gl::Texture3D* BuildMipMappedTexture (gl::Texture3D* tex_vol,
                                        gl::Texture1D* tex_tf,
                                        glm::vec3 volume_voxel_size);

  float GetBaseLevelGaussianSigma0 ();
  void SetBaseLevelGaussianSigma0 (float basegaussianlevel);
  float* GetBaseLevelGaussianSigma0Ptr ();

  bool IsUsingCustomExtCoefVolumeResolution ();
  void UseCustomExtCoefVolumeResolution (bool f);
  bool* IsUsingCustomExtCoefVolumeResolutionPtr ();

  glm::ivec3 GetCustomExtCoefVolumeResolution ();
  void SetCustomExtCoefVolumeResolution (glm::ivec3 vol_res);
  void SetCustomExtCoefVolumeResolution (int v_w, int v_h, int v_d);
  glm::ivec3* GetCustomExtCoefVolumeResolutionPtr ();

protected:

private:
  gl::Texture3D* GenerateExtinctionCoefficientVolumeSameSize (gl::Texture3D* tex_vol, gl::Texture1D* ttf, glm::vec3 VoxelSize);
  gl::Texture3D* GenerateExtinctionCoefficientVolumeAnySize (gl::Texture3D* tex_vol, gl::Texture1D* ttf, glm::vec3 VolumeGridSize);
  
  // Convert all opacity levels to extinction coefficients
  void TransformTexOpacityToExtinction (gl::Texture3D* gaussian_volume, int n_computed_mmaps);
  
  float base_level_sigma0;

  bool map_specific_volume_resolution;
  glm::ivec3 base_level_volume_resolution;
};

#endif