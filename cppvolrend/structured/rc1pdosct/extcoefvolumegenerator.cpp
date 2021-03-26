#include "../../defines.h"
#include "extcoefvolumegenerator.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <gl_utils/computeshader.h>

ExtinctionCoefficientVolume::ExtinctionCoefficientVolume ()
{
  SetBaseLevelGaussianSigma0(1.0f);
  UseCustomExtCoefVolumeResolution(true);
  SetCustomExtCoefVolumeResolution(128, 128, 128);
}

ExtinctionCoefficientVolume::~ExtinctionCoefficientVolume ()
{}

gl::Texture3D* ExtinctionCoefficientVolume::BuildMipMappedTexture (gl::Texture3D* tex_vol, gl::Texture1D* tex_tf, glm::vec3 volume_voxel_size)
{
  gl::Texture3D* tex3d = nullptr;

  if (IsUsingCustomExtCoefVolumeResolution())
  {
    glm::vec3 VolumeGridSize = glm::vec3(float(tex_vol->GetWidth())  * volume_voxel_size.x,
                                         float(tex_vol->GetHeight()) * volume_voxel_size.y,
                                         float(tex_vol->GetDepth())  * volume_voxel_size.z);

    tex3d = GenerateExtinctionCoefficientVolumeAnySize(tex_vol, tex_tf, VolumeGridSize);
  }
  else
  {
    tex3d = GenerateExtinctionCoefficientVolumeSameSize(tex_vol, tex_tf, volume_voxel_size);
  }

  if(tex_tf) delete tex_tf;
  
  return tex3d;
}

float ExtinctionCoefficientVolume::GetBaseLevelGaussianSigma0 ()
{
  return base_level_sigma0;
}

void ExtinctionCoefficientVolume::SetBaseLevelGaussianSigma0 (float basegaussianlevel)
{
  base_level_sigma0 = basegaussianlevel;
}

float* ExtinctionCoefficientVolume::GetBaseLevelGaussianSigma0Ptr ()
{
  return (&base_level_sigma0);
}

bool ExtinctionCoefficientVolume::IsUsingCustomExtCoefVolumeResolution ()
{
  return map_specific_volume_resolution;
}

void ExtinctionCoefficientVolume::UseCustomExtCoefVolumeResolution (bool f)
{
  map_specific_volume_resolution = f;
}

bool* ExtinctionCoefficientVolume::IsUsingCustomExtCoefVolumeResolutionPtr ()
{
  return (&map_specific_volume_resolution);
}

glm::ivec3 ExtinctionCoefficientVolume::GetCustomExtCoefVolumeResolution ()
{
  return base_level_volume_resolution;
}

void ExtinctionCoefficientVolume::SetCustomExtCoefVolumeResolution (glm::ivec3 vol_res)
{
  base_level_volume_resolution = vol_res;
}

void ExtinctionCoefficientVolume::SetCustomExtCoefVolumeResolution (int v_w, int v_h, int v_d)
{
  SetCustomExtCoefVolumeResolution(glm::ivec3(v_w, v_h, v_d));
}

glm::ivec3* ExtinctionCoefficientVolume::GetCustomExtCoefVolumeResolutionPtr ()
{
  return (&base_level_volume_resolution);
}

gl::Texture3D* ExtinctionCoefficientVolume::GenerateExtinctionCoefficientVolumeSameSize (gl::Texture3D* tex_vol, gl::Texture1D* ttf, glm::vec3 voxel_size)
{
  gl::Texture3D* tex3d = nullptr;

  ////////////////////////////////////////////////////////////////////////////////////
  // 1. Compute level 0 of the extinction coefficient volume 
  // Initialize compute shader
  gl::ComputeShader* cpshader = new gl::ComputeShader();
  cpshader->SetShaderFile(CPPVOLREND_DIR"structured/rc1pdosct/glslextgen/gen_extcoefvol_samesize.comp");
  cpshader->LoadAndLink();
  cpshader->Bind(); 

  glActiveTexture(GL_TEXTURE0);

  // Initialize extinction coefficient volume texture 3D
  tex3d = new gl::Texture3D(tex_vol->GetWidth(), tex_vol->GetHeight(), tex_vol->GetDepth());

  // Set initial texture parameters
  tex3d->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);

  // Set default data
  tex3d->SetData(NULL, GL_R16F, GL_RED, GL_FLOAT);

  // First, evaluate for level 0.0
  // 3D texture: layered must be true (???)
  // https://stackoverflow.com/questions/17015132/compute-shader-not-modifying-3d-texture
  // https://stackoverflow.com/questions/37136813/what-is-the-difference-between-glbindimagetexture-and-glbindtexture
  // https://www.khronos.org/opengl/wiki/GLAPI/glBindImageTexture
  //// Bind our input texture 3D
  //glBindTexture(GL_TEXTURE_3D, tex3d->GetTextureID());
  cpshader->BindImageTexture(tex3d, 0, 0, GL_WRITE_ONLY, GL_R16F);

  // Bind volume texture
  cpshader->SetUniformTexture3D("TexInputVolume", tex_vol->GetTextureID(), 1);

  // Bind transfer function texture
  cpshader->SetUniformTexture1D("TexInputTransferFunc", ttf->GetTextureID(), 2);

  // Upload volume dimensions
  cpshader->SetUniform("VolumeResolution", glm::vec3((float)tex_vol->GetWidth(), (float)tex_vol->GetHeight(), (float)tex_vol->GetDepth()));

  // Upload volume scales
  cpshader->SetUniform("VoxelSize", voxel_size);

  // Upload base standard deviation
  cpshader->SetUniform("S0", GetBaseLevelGaussianSigma0());

  cpshader->BindUniforms();

  cpshader->RecomputeNumberOfGroups(tex_vol->GetWidth(), tex_vol->GetHeight(), tex_vol->GetDepth());

  glActiveTexture(GL_TEXTURE0);
  cpshader->Dispatch();

  // Auto mipmap
  glBindTexture(GL_TEXTURE_3D, tex3d->GetTextureID());
  glGenerateMipmap(GL_TEXTURE_3D);

  glBindTexture(GL_TEXTURE_3D, 0);

  cpshader->Unbind();
  delete cpshader;

  ////////////////////////////////////////////////////////////////////////////////////
  // 2. Compute Manual mipmap
  // Manually generate mip map levels
  // Initialize compute shader
  gl::ComputeShader* cpshaderlevel = new gl::ComputeShader();
  cpshaderlevel->SetShaderFile(CPPVOLREND_DIR"structured/rc1pdosct/glslextgen/gen_extcoefvol_samesize_mmlevel.comp");
  cpshaderlevel->LoadAndLink();
  cpshaderlevel->Bind();

  // Upload base standard deviation
  cpshaderlevel->SetUniform("S0", GetBaseLevelGaussianSigma0());
  cpshaderlevel->BindUniform("S0");

  glm::vec3 VolumeGridSize(voxel_size.x * (float)tex_vol->GetWidth(),
                           voxel_size.y * (float)tex_vol->GetHeight(),
                           voxel_size.z * (float)tex_vol->GetDepth());
  
  // Volume size, based on the grid min max anchor
  cpshaderlevel->SetUniform("VolumeGridSize", VolumeGridSize);
  cpshaderlevel->BindUniform("VolumeGridSize");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, tex3d->GetTextureID());

  // Get texture dimension sizes
  int max_level, number_of_computed_levels;
  glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, &max_level);

  // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetTexLevelParameter.xhtml
  // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetTexImage.xhtml
  for (int i = 1; i < max_level; i++)
  {
    int wd, ht, dp;
    glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_WIDTH,  &wd);
    glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_HEIGHT, &ht);
    glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_DEPTH,  &dp);

    if (wd * ht * dp == 0) break;
    number_of_computed_levels = i;

    // Evaluate for level i
    // The first value, unit, is the current binding index in the compute shader
    cpshaderlevel->BindImageTexture(tex3d, 0, i, GL_WRITE_ONLY, GL_R16F);

    // Bind and Upload Gaussian Filter Kernel Texture
    glUniform1i(glGetUniformLocation(cpshaderlevel->GetProgramID(), "TexExtinctionCoefficientVolume"), 0);

    cpshaderlevel->SetUniform("PreviousMipMapLevel", (float)(i - 1));
    cpshaderlevel->BindUniform("PreviousMipMapLevel");

    cpshaderlevel->SetUniform("SubLevelVolumeResolution", glm::vec3(wd, ht, dp));
    cpshaderlevel->BindUniform("SubLevelVolumeResolution");

    cpshaderlevel->SetUniform("Si", GetBaseLevelGaussianSigma0() * glm::pow(2.0f, (float)i));
    cpshaderlevel->BindUniform("Si");

    cpshaderlevel->RecomputeNumberOfGroups(wd, ht, dp);
    cpshaderlevel->Dispatch();
  }
  cpshaderlevel->Unbind();
  delete cpshaderlevel;

  ////////////////////////////////////////////////////////////////////////////////////
  // 3. Transform opacities back to extinction coefficients in compute shader
  TransformTexOpacityToExtinction(tex3d, number_of_computed_levels);
  ////////////////////////////////////////////////////////////////////////////////////  
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, 0);

  std::cout << "ExtinctionCoefficientVolume: " << number_of_computed_levels << " computed mipmap levels..." << std::endl;
  gl::ExitOnGLError("ExtinctionCoefficientVolume: Error after generating the transparency volume.");
  return tex3d;
}

gl::Texture3D* ExtinctionCoefficientVolume::GenerateExtinctionCoefficientVolumeAnySize (gl::Texture3D* tex_vol, gl::Texture1D* tex_tf, glm::vec3 VolumeGridSize)
{
  gl::Texture3D* tex3d = nullptr;
  glm::vec3 base_level_voxel_sizes = VolumeGridSize / glm::vec3(GetCustomExtCoefVolumeResolution());
 
  ////////////////////////////////////////////////////////////////////////////////////
  // 1. Compute level 0 of the extinction coefficient volume 
  // Initialize compute shader
  gl::ComputeShader* cpshader = new gl::ComputeShader();
  cpshader->SetShaderFile(CPPVOLREND_DIR"structured/rc1pdosct/glslextgen/gen_extcoefvol_anysize.comp");
  cpshader->LoadAndLink();
  cpshader->Bind();
  
  glActiveTexture(GL_TEXTURE0);
  
  // Initialize extinction coefficient volume texture 3D
  tex3d = new gl::Texture3D(GetCustomExtCoefVolumeResolution());
  
  // Set initial texture parameters
  tex3d->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
  
  // Set default data
  tex3d->SetData(NULL, GL_R16F, GL_RED, GL_FLOAT);
  
  // First, evaluate for level 0.0
  // 3D texture: layered must be true (???)
  // https://stackoverflow.com/questions/17015132/compute-shader-not-modifying-3d-texture
  // https://stackoverflow.com/questions/37136813/what-is-the-difference-between-glbindimagetexture-and-glbindtexture
  // https://www.khronos.org/opengl/wiki/GLAPI/glBindImageTexture
  // Bind our input texture 3D
  //glBindTexture(GL_TEXTURE_3D, tex3d->GetTextureID());
  cpshader->BindImageTexture(tex3d, 0, 0, GL_WRITE_ONLY, GL_R16F);

  // Bind volume texture
  cpshader->SetUniformTexture3D("TexInputVolume", tex_vol->GetTextureID(), 1);

  // Bind transfer function texture
  cpshader->SetUniformTexture1D("TexInputTransferFunc", tex_tf->GetTextureID(), 2);
  
  // Upload volume dimensions
  cpshader->SetUniform("ExtCoefVolumeResolution", glm::vec3(GetCustomExtCoefVolumeResolution()));
    
  // Upload volume voxel size
  cpshader->SetUniform("ExtCoefVoxelSize", base_level_voxel_sizes);

  // Upload base standard deviation
  cpshader->SetUniform("S0", GetBaseLevelGaussianSigma0());
  
  // Volume size, based on the grid min max anchor
  cpshader->SetUniform("VolumeGridSize", VolumeGridSize);

  cpshader->BindUniforms();
  cpshader->RecomputeNumberOfGroups(GetCustomExtCoefVolumeResolution().x,
                                    GetCustomExtCoefVolumeResolution().y,
                                    GetCustomExtCoefVolumeResolution().z);
  
  glActiveTexture(GL_TEXTURE0);
  cpshader->Dispatch();
  
  // Auto mipmap
  glBindTexture(GL_TEXTURE_3D, tex3d->GetTextureID());
  glGenerateMipmap(GL_TEXTURE_3D);
  
  glBindTexture(GL_TEXTURE_3D, 0);
  
  cpshader->Unbind();
  delete cpshader;
  
  ////////////////////////////////////////////////////////////////////////////////////
  // 2. Compute Manual mipmap
  // Initialize compute shader
  gl::ComputeShader* cpshaderlevel = new gl::ComputeShader();
  cpshaderlevel->SetShaderFile(CPPVOLREND_DIR"structured/rc1pdosct/glslextgen/gen_extcoefvol_anysize_mmlevel.comp");
  cpshaderlevel->LoadAndLink();
  cpshaderlevel->Bind();
  
  // Upload base standard deviation
  cpshaderlevel->SetUniform("S0", GetBaseLevelGaussianSigma0());
  cpshaderlevel->BindUniform("S0");

  // Volume size, based on the grid min max anchor
  cpshaderlevel->SetUniform("VolumeGridSize", VolumeGridSize);
  cpshaderlevel->BindUniform("VolumeGridSize");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, tex3d->GetTextureID());
  
  // Get texture dimension sizes
  int max_level, number_of_computed_levels;
  glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, &max_level);
  
  // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetTexLevelParameter.xhtml
  // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetTexImage.xhtml
  for (int i = 1; i < max_level; i++)
  {
    int wd, ht, dp;
    glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_WIDTH, &wd);
    glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_HEIGHT, &ht);
    glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_DEPTH, &dp);
    //std::cout << "Level " << i << ": " << wd << " " << ht << " " << dp << " " << std::endl;
  
    if (wd * ht * dp == 0) break;
    number_of_computed_levels = i;
  
    // Evaluate for level i
    // The first value, unit, is the current binding index in the compute shader
    cpshaderlevel->BindImageTexture(tex3d, 0, i, GL_WRITE_ONLY, GL_R16F);
  
    // Bind and Upload Gaussian Filter Kernel Texture
    glUniform1i(glGetUniformLocation(cpshaderlevel->GetProgramID(), "TexExtinctionCoefficientVolume"), 0);
  
    cpshaderlevel->SetUniform("PreviousMipMapLevel", (float)(i - 1));
    cpshaderlevel->BindUniform("PreviousMipMapLevel");
  
    cpshaderlevel->SetUniform("SubLevelVolumeResolution", glm::vec3(wd, ht, dp));
    cpshaderlevel->BindUniform("SubLevelVolumeResolution");

    cpshaderlevel->SetUniform("Si", GetBaseLevelGaussianSigma0() * glm::pow(2.0f, (float)i));
    cpshaderlevel->BindUniform("Si");
  
    cpshaderlevel->RecomputeNumberOfGroups(wd, ht, dp);
    cpshaderlevel->Dispatch();
  }
  cpshaderlevel->Unbind();
  delete cpshaderlevel;
  
  ////////////////////////////////////////////////////////////////////////////////////
  // 3. Transform opacities back to extinction coefficients in compute shader
  TransformTexOpacityToExtinction(tex3d, number_of_computed_levels);
  ////////////////////////////////////////////////////////////////////////////////////  
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, 0);
  
  std::cout << "ExtinctionCoefficientVolume: " << number_of_computed_levels << " computed mipmap levels..." << std::endl;
  gl::ExitOnGLError("ExtinctionCoefficientVolume: Error after generating the transparency volume.");
  return tex3d;
}

void ExtinctionCoefficientVolume::TransformTexOpacityToExtinction (gl::Texture3D* input_tex3d , int n_computed_mmaps)
{
  gl::ComputeShader* cpshaderbacktau = new gl::ComputeShader();
  cpshaderbacktau->SetShaderFile(CPPVOLREND_DIR"structured/rc1pdosct/glslextgen/backtotau.comp");
  cpshaderbacktau->LoadAndLink();
  cpshaderbacktau->Bind();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, input_tex3d->GetTextureID());
  
  for (int i = 0; i <= n_computed_mmaps; i++)
  {
    // Get Volume dimensions at level i
    int wd, ht, dp;
    glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_WIDTH,  &wd);
    glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_HEIGHT, &ht);
    glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_DEPTH,  &dp);

    // Bind current level i of the Extinction coefficient volume
    glBindImageTexture(0, input_tex3d->GetTextureID(), i, GL_TRUE, 0, GL_READ_WRITE, GL_R16F);

    // Upload current volume dimension at level i
    cpshaderbacktau->SetUniform("MMLevelVolResolution", glm::vec3((float)wd, (float)ht, (float)dp));
    cpshaderbacktau->BindUniform("MMLevelVolResolution");

    // Upload base standard deviation
    cpshaderbacktau->SetUniform("MMLevel", i);
    cpshaderbacktau->BindUniform("MMLevel");

    // Upload base standard deviation
    cpshaderbacktau->SetUniform("S0", GetBaseLevelGaussianSigma0());
    cpshaderbacktau->BindUniform("S0");

    cpshaderbacktau->RecomputeNumberOfGroups(wd, ht, dp);
    cpshaderbacktau->Dispatch();
  }

  cpshaderbacktau->Unbind();
  delete cpshaderbacktau;
}