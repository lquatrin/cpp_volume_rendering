/**
 * Class used to build an additional volume, storing lighting info.
 *
 * When active, the light cache is preprocessed and fecthed in the rendering pass.
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef PREILLUMINATION_LIGHT_CACHE_VOLUME_H
#define PREILLUMINATION_LIGHT_CACHE_VOLUME_H

#include <glm/glm.hpp>
#include <gl_utils/texture3d.h>

class PreIlluminationStructuredVolume
{
public:
  PreIlluminationStructuredVolume (int n_channels = 2);
  ~PreIlluminationStructuredVolume ();

  bool IsActive ();
  void SetActive (bool f);

  glm::ivec3 GetLightCacheResolution ();
  void SetLightCacheResolution (glm::ivec3 tex_resolution);
  void SetLightCacheResolution (int w, int h, int d);

  gl::Texture3D* GetLightCacheTexturePointer ();
  void GenerateLightCacheTexture ();
  void DestroyLightCacheTexture ();

  // returns a bvec2:
  // x - active changed
  // y - resolution changed
  glm::bvec2 SetImGuiComponents ();

protected:

private:
  bool m_active;
  glm::ivec3 m_light_cache_resolution;
  gl::Texture3D* m_tex_glsl_light_vol_cache;
  
  GLint m_tex_internal_format;
  GLint m_tex_data_format;
  GLint m_tex_data_type;
};

#endif