#include "preprocessingstages.h"

VCTPreProcessing::VCTPreProcessing ()
{
  use_glsl_to_precompute_data = false;

  glsl_supervoxel_meanstddev = nullptr;
  glsl_preintegration_lookup = nullptr;

  tree_spr_voxel.clear();
}

VCTPreProcessing::~VCTPreProcessing()
{
}

double VCTPreProcessing::GetMeanFromSuperVoxel (int lvl, int lw, int lh, int ld, int vw, int vh, int vd)
{
  int x = glm::clamp(lw, 0, vw);
  int y = glm::clamp(lh, 0, vh);
  int z = glm::clamp(ld, 0, vd);

  return tree_spr_voxel[lvl]->sv_data[x + (y * vw) + (z * vw * vh)].mean;
}

double VCTPreProcessing::GetStdDevFromSuperVoxel (int lvl, int lw, int lh, int ld, int vw, int vh, int vd)
{
  int x = glm::clamp(lw, 0, vw);
  int y = glm::clamp(lh, 0, vh);
  int z = glm::clamp(ld, 0, vd);

  return tree_spr_voxel[lvl]->sv_data[x + (y * vw) + (z * vw * vh)].stdv;
}

void VCTPreProcessing::PreProcessSuperVoxels (vis::StructuredGridVolume* vol)
{
  if (use_glsl_to_precompute_data)
  {
    //glsl_supervoxel_meanstddev = GLSLPreComputeSuperVoxels();
    //maximum_standard_deviation = 255.0;
  }

  int w = vol->GetWidth();
  int h = vol->GetHeight();
  int d = vol->GetDepth();

  tree_spr_voxel.push_back(new SuperVoxelLevel(glm::ivec3(w, h, d)));
  for (int x = 0; x < w; x++)
  {
    for (int y = 0; y < h; y++)
    {
      for (int z = 0; z < d; z++)
      {
        tree_spr_voxel[0]->sv_data[x + (y * w) + (z * w * h)].mean = vol->GetNormalizedSample(x,y,z) * 255.0;
        tree_spr_voxel[0]->sv_data[x + (y * w) + (z * w * h)].stdv = 0.0;
      }
    }
  }

  double max_stddev = 0.0;

  int vw = w;
  int vh = h;
  int vd = d;
  int mm_level = 1;
  w = w / 2;
  h = h / 2;
  d = d / 2;
  while (w * h * d >= 1)
  {
    tree_spr_voxel.push_back(new SuperVoxelLevel(glm::ivec3(w, h, d)));

    for (int iw = 0; iw < w; iw++)
    {
      for (int ih = 0; ih < h; ih++)
      {
        for (int id = 0; id < d; id++)
        {
          int lw = iw * 2;
          int lh = ih * 2;
          int ld = id * 2;

          double vm0 = GetMeanFromSuperVoxel(mm_level - 1, lw    , lh    , ld    , vw, vh, vd);
          double vm1 = GetMeanFromSuperVoxel(mm_level - 1, lw    , lh    , ld + 1, vw, vh, vd);
          double vm2 = GetMeanFromSuperVoxel(mm_level - 1, lw    , lh + 1, ld    , vw, vh, vd);
          double vm3 = GetMeanFromSuperVoxel(mm_level - 1, lw    , lh + 1, ld + 1, vw, vh, vd);
          double vm4 = GetMeanFromSuperVoxel(mm_level - 1, lw + 1, lh    , ld    , vw, vh, vd);
          double vm5 = GetMeanFromSuperVoxel(mm_level - 1, lw + 1, lh    , ld + 1, vw, vh, vd);
          double vm6 = GetMeanFromSuperVoxel(mm_level - 1, lw + 1, lh + 1, ld    , vw, vh, vd);
          double vm7 = GetMeanFromSuperVoxel(mm_level - 1, lw + 1, lh + 1, ld + 1, vw, vh, vd);
          double vmn = (vm0 + vm1 + vm2 + vm3 + vm4 + vm5 + vm6 + vm7) / 8.0;

          tree_spr_voxel[mm_level]->sv_data[iw + (ih * w) + (id * w * h)].mean = vmn;

          double vstdd = glm::sqrt(
            (pow(vm0 - vmn, 2.0)
              + pow(vm1 - vmn, 2.0)
              + pow(vm2 - vmn, 2.0)
              + pow(vm3 - vmn, 2.0)
              + pow(vm4 - vmn, 2.0)
              + pow(vm5 - vmn, 2.0)
              + pow(vm6 - vmn, 2.0)
              + pow(vm7 - vmn, 2.0)) / 8.0
          );

          tree_spr_voxel[mm_level]->sv_data[iw + (ih * w) + (id * w * h)].stdv = vstdd;

          max_stddev = glm::max(vstdd, max_stddev);
        }
      }
    }

    vw = w;
    vh = h;
    vd = d;
    mm_level++;
    w = w / 2;
    h = h / 2;
    d = d / 2;
  }

  glsl_supervoxel_meanstddev = new gl::Texture3D(vol->GetWidth(), vol->GetHeight(), vol->GetDepth());
  glsl_supervoxel_meanstddev->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);

  // Set the data of each mipmap level to then update to glsl shader
  for (int i = 0; i < mm_level; i++)
  {
    int w = tree_spr_voxel[i]->dim.x;
    int h = tree_spr_voxel[i]->dim.y;
    int d = tree_spr_voxel[i]->dim.z;
        
    GLfloat* sdata = new GLfloat[w*h*d * 2];
    for (int v = 0; v < w*h*d; v++)
    {
      sdata[v * 2 + 0] = tree_spr_voxel[i]->sv_data[v].mean;
      sdata[v * 2 + 1] = tree_spr_voxel[i]->sv_data[v].stdv;
    }

    glTexImage3D(GL_TEXTURE_3D, i, GL_RG16F, w, h, d, 0, GL_RG, GL_FLOAT, sdata);
    delete[] sdata;
  }

  maximum_standard_deviation = max_stddev;
  printf("Super Voxels Computed! Maximum Standard Deviation %g\n", max_stddev);
}

double VCTPreProcessing::GaussianEvaluation (double x, double mean, double stddev)
{
  double normalization_factor = 1.0 / (stddev * glm::sqrt(2.0 * glm::pi<double>()));
  return normalization_factor * glm::exp(-((x - mean)*(x - mean)) / (2.0*stddev*stddev));
}

double VCTPreProcessing::OpacityGaussianEvaluation (double mean, double stddev, vis::StructuredGridVolume* vol, vis::TransferFunction* tf)
{
  int dens_val = vol->GetMaxDensity();
  double SumG = 0.0;
  double SumW = 0.0;

  if (fabs(stddev) > 0.0001)
  {
    for (int i = 0; i < dens_val; i++)
    {
      double W = GaussianEvaluation(i, mean, stddev);
      SumG += W * tf->GetOpc(i, dens_val);
      SumW += W;
    }
    SumG = SumG / SumW;
  }
  else
    SumG = tf->GetOpc(mean, dens_val);

  return SumG;
}

void VCTPreProcessing::PreProcessPreIntegrationTable (vis::StructuredGridVolume* vol, vis::TransferFunction* tf)
{
  if (use_glsl_to_precompute_data)
  {
    //glsl_preintegration_lookup = GLSLPreComputePreIntegrationTable();
  }

  double dens_val = vol->GetMaxDensity();
  int w = glm::ceil(dens_val);
  int h = glm::ceil(maximum_standard_deviation);

  GLfloat* preintegrationvalues = new GLfloat[w * h];
  for (int iw = 0; iw < w; iw++)
  {
    for (int ih = 0; ih < h; ih++)
    {
      preintegrationvalues[iw + (ih * w)] = (float)OpacityGaussianEvaluation(iw, ih, vol, tf);
    }
  }

  glsl_preintegration_lookup = new gl::Texture2D(w, h);
  glsl_preintegration_lookup->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
  glsl_preintegration_lookup->SetData(preintegrationvalues, GL_R16F, GL_RED, GL_FLOAT);

  delete[] preintegrationvalues;

  gl::ExitOnGLError("ERROR: After SetData");
}

gl::Texture3D* VCTPreProcessing::GLSLPreComputeSuperVoxels ()
{
  //glm::vec3 vol_scale = glm::vec3(vol->GetScaleX(),
  //                      vol->GetScaleY(),
  //                      vol->GetScaleZ());
  //
  //// Initialize compute shader
  //gl::ComputeShader* cpsupervoxelbase = new gl::ComputeShader();
  //cpsupervoxelbase->SetShaderFile("shader/voxelconetracinggaussian/supervoxel_base.comp");
  //cpsupervoxelbase->LoadAndLink();
  //cpsupervoxelbase->Bind();
  //
  //glActiveTexture(GL_TEXTURE1);
  //
  //int w = vol->GetWidth();
  //int h = vol->GetHeight();
  //int d = vol->GetDepth();
  //
  //gl::GLTexture3D* tex_supervoxel = new gl::GLTexture3D(w, h, d);
  //tex_supervoxel->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
  //tex_supervoxel->SetData(NULL, GL_RG16F, GL_RG, GL_FLOAT);
  //
  //glBindTexture(GL_TEXTURE_3D, tex_supervoxel->GetTextureID());
  //
  //glBindImageTexture(1, tex_supervoxel->GetTextureID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG16F);
  //
  //// Bind volume texture
  //glActiveTexture(GL_TEXTURE0);
  //glBindTexture(GL_TEXTURE_3D, glsl_volume->GetTextureID());
  //glUniform1i(glGetUniformLocation(cpsupervoxelbase->GetProgramID(), "TexVolume"), 0);
  //
  //// Upload volume dimensions
  //glUniform3f(
  //  glGetUniformLocation(cpsupervoxelbase->GetProgramID(), "VolumeDimensions")
  //  , (float)vol->GetWidth(), (float)vol->GetHeight(), (float)vol->GetDepth()
  //);
  //
  //// Upload volume scales
  //glUniform3f(
  //  glGetUniformLocation(cpsupervoxelbase->GetProgramID(), "VolumeScales")
  //  , vol->GetScaleX(), vol->GetScaleY(), vol->GetScaleZ()
  //);
  //
  //// Upload volume scales
  //glUniform1f(
  //  glGetUniformLocation(cpsupervoxelbase->GetProgramID(), "MaxDensityVolume")
  //  , vol->GetMaxDensity()
  //);
  //
  //{
  //  // x
  //  int disp_w = 2;
  //  while (disp_w < vol->GetWidth()) disp_w = disp_w * 2;
  //  int num_groups_x = disp_w / 8;
  //
  //  // y
  //  int disp_h = 2;
  //  while (disp_h < vol->GetHeight()) disp_h = disp_h * 2;
  //  int num_groups_y = disp_h / 8;
  //
  //  // z
  //  int disp_d = 2;
  //  while (disp_d < vol->GetDepth()) disp_d = disp_d * 2;
  //  int num_groups_z = disp_d / 8;
  //
  //  glActiveTexture(GL_TEXTURE1);
  //  glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
  //}
  //
  //cpsupervoxelbase->Unbind();
  //delete cpsupervoxelbase;
  //
  //// Initialize compute shader
  //gl::ComputeShader* cpsupervoxellevel = new gl::ComputeShader();
  //cpsupervoxellevel->SetShaderFile("shader/voxelconetracinggaussian/supervoxel_level.comp");
  //cpsupervoxellevel->LoadAndLink();
  //cpsupervoxellevel->Bind();
  //
  //glActiveTexture(GL_TEXTURE1);
  //glBindTexture(GL_TEXTURE_3D, tex_supervoxel->GetTextureID());
  //
  //// Get the current dimensions and the number of mipmaplevels
  //int wd_b, ht_b, dp_b;
  //glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &wd_b);
  //glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &ht_b);
  //glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &dp_b);
  //std::cout << "Level 0" << ": " << wd_b << " " << ht_b << " " << dp_b << " " << std::endl;
  //
  //// Get the max level of texture 3D
  //int max_level;
  //glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, &max_level);
  //
  //for (int i = 1; i < max_level; i++)
  //{
  //  // Get Current volume dimension
  //  int wd_l, ht_l, dp_l;
  //  glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_WIDTH, &wd_l);
  //  glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_HEIGHT, &ht_l);
  //  glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_DEPTH, &dp_l);
  //  std::cout << "Level " << i << ": " << wd_l << " " << ht_l << " " << dp_l << " " << std::endl;
  //
  //  if (wd_l * ht_l * dp_l == 0) break;
  //
  //  // Evaluate for level i
  //  glBindImageTexture(1, tex_supervoxel->GetTextureID(), i, GL_TRUE, 0, GL_WRITE_ONLY, GL_R16F);
  //
  //  // Bind opacity volume texture
  //  glActiveTexture(GL_TEXTURE0);
  //  glBindTexture(GL_TEXTURE_3D, tex_supervoxel->GetTextureID());
  //  glUniform1i(glGetUniformLocation(cpsupervoxellevel->GetProgramID(), "TexSuperVoxelVolume"), 0);
  //
  //  // Upload volume dimensions
  //  glUniform3f(
  //    glGetUniformLocation(cpsupervoxellevel->GetProgramID(), "VolumeDimensionsBase")
  //    , (float)wd_b, (float)ht_b, (float)dp_b
  //  );
  //
  //  glUniform3f(
  //    glGetUniformLocation(cpsupervoxellevel->GetProgramID(), "VolumeDimensionsLevel")
  //    , (float)wd_l, (float)ht_l, (float)dp_l
  //  );
  //
  //  // Current mip map level being used
  //  glUniform1f(
  //    glGetUniformLocation(cpsupervoxellevel->GetProgramID(), "MipMapLevel")
  //    , (float)i
  //  );
  //
  //  // Current mip map level being used
  //  glUniform1i(
  //    glGetUniformLocation(cpsupervoxellevel->GetProgramID(), "PositionMultiplier")
  //    , (int)pow(2, i)
  //  );
  //
  //  // x
  //  int disp_w = 2;
  //  while (disp_w < wd_l) disp_w = disp_w * 2;
  //  if (disp_w < 8) disp_w = 8;
  //  int num_groups_x = disp_w / 8;
  //
  //  // y
  //  int disp_h = 2;
  //  while (disp_h < ht_l) disp_h = disp_h * 2;
  //  if (disp_h < 8) disp_h = 8;
  //  int num_groups_y = disp_h / 8;
  //
  //  // z
  //  int disp_d = 2;
  //  while (disp_d < dp_l) disp_d = disp_d * 2;
  //  if (disp_d < 8) disp_d = 8;
  //  int num_groups_z = disp_d / 8;
  //
  //  glActiveTexture(GL_TEXTURE1);
  //  // dispatch for level i
  //  glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
  //}
  //
  //glActiveTexture(GL_TEXTURE0);
  //glBindTexture(GL_TEXTURE_3D, 0);
  //
  //gl::ExitOnGLError("ERROR: After SetData");
  return nullptr;
  //return tex_supervoxel;
}

gl::Texture2D* VCTPreProcessing::GLSLPreComputePreIntegrationTable ()
{
  return nullptr;
  //vr::Volume* vol = data_manager->GetCurrentVolume();
  //
  //vol_scale = glm::vec3(data_manager->GetCurrentVolume()->GetScaleX(),
  //  data_manager->GetCurrentVolume()->GetScaleY(),
  //  data_manager->GetCurrentVolume()->GetScaleZ());
  //
  //// Initialize compute shader
  //gl::ComputeShader* cpsupervoxelbase = new gl::ComputeShader();
  //cpsupervoxelbase->SetShaderFile("shader/voxelconetracinggaussian/supervoxel_base.comp");
  //cpsupervoxelbase->LoadAndLink();
  //cpsupervoxelbase->Bind();
  //
  //glActiveTexture(GL_TEXTURE1);
  //
  //int w = data_manager->GetCurrentVolume()->GetWidth();
  //int h = data_manager->GetCurrentVolume()->GetHeight();
  //int d = data_manager->GetCurrentVolume()->GetDepth();
  //
  //gl::GLTexture3D* tex_supervoxel = new gl::GLTexture3D(w, h, d);
  //tex_supervoxel->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, true);
  //tex_supervoxel->SetData(NULL, GL_RG16F, GL_RG, GL_FLOAT);
  //
  //glBindTexture(GL_TEXTURE_3D, tex_supervoxel->GetTextureID());
  //
  //glBindImageTexture(1, tex_supervoxel->GetTextureID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RG16F);
  //
  //// Bind volume texture
  //glActiveTexture(GL_TEXTURE0);
  //glBindTexture(GL_TEXTURE_3D, glsl_volume->GetTextureID());
  //glUniform1i(glGetUniformLocation(cpsupervoxelbase->GetProgramID(), "TexVolume"), 0);
  //
  //// Upload volume dimensions
  //glUniform3f(
  //  glGetUniformLocation(cpsupervoxelbase->GetProgramID(), "VolumeDimensions")
  //  , (float)vol->GetWidth(), (float)vol->GetHeight(), (float)vol->GetDepth()
  //);
  //
  //// Upload volume scales
  //glUniform3f(
  //  glGetUniformLocation(cpsupervoxelbase->GetProgramID(), "VolumeScales")
  //  , vol->GetScaleX(), vol->GetScaleY(), vol->GetScaleZ()
  //);
  //
  //// Upload volume scales
  //glUniform1f(
  //  glGetUniformLocation(cpsupervoxelbase->GetProgramID(), "MaxDensityVolume")
  //  , vol->GetMaxDensity()
  //);
  //
  //{
  //  // x
  //  int disp_w = 2;
  //  while (disp_w < vol->GetWidth()) disp_w = disp_w * 2;
  //  int num_groups_x = disp_w / 8;
  //
  //  // y
  //  int disp_h = 2;
  //  while (disp_h < vol->GetHeight()) disp_h = disp_h * 2;
  //  int num_groups_y = disp_h / 8;
  //
  //  // z
  //  int disp_d = 2;
  //  while (disp_d < vol->GetDepth()) disp_d = disp_d * 2;
  //  int num_groups_z = disp_d / 8;
  //
  //  glActiveTexture(GL_TEXTURE1);
  //  glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
  //}
  //
  //cpsupervoxelbase->Unbind();
  //delete cpsupervoxelbase;
  //
  //// Initialize compute shader
  //gl::ComputeShader* cpsupervoxellevel = new gl::ComputeShader();
  //cpsupervoxellevel->SetShaderFile("shader/voxelconetracinggaussian/supervoxel_level.comp");
  //cpsupervoxellevel->LoadAndLink();
  //cpsupervoxellevel->Bind();
  //
  //glActiveTexture(GL_TEXTURE1);
  //glBindTexture(GL_TEXTURE_3D, tex_supervoxel->GetTextureID());
  //
  //// Get the current dimensions and the number of mipmaplevels
  //int wd_b, ht_b, dp_b;
  //glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &wd_b);
  //glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &ht_b);
  //glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &dp_b);
  //std::cout << "Level 0" << ": " << wd_b << " " << ht_b << " " << dp_b << " " << std::endl;
  //
  //// Get the max level of texture 3D
  //int max_level;
  //glGetTexParameteriv(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, &max_level);
  //
  //for (int i = 1; i < max_level; i++)
  //{
  //  // Get Current volume dimension
  //  int wd_l, ht_l, dp_l;
  //  glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_WIDTH, &wd_l);
  //  glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_HEIGHT, &ht_l);
  //  glGetTexLevelParameteriv(GL_TEXTURE_3D, i, GL_TEXTURE_DEPTH, &dp_l);
  //  std::cout << "Level " << i << ": " << wd_l << " " << ht_l << " " << dp_l << " " << std::endl;
  //
  //  if (wd_l * ht_l * dp_l == 0) break;
  //
  //  // Evaluate for level i
  //  glBindImageTexture(1, tex_supervoxel->GetTextureID(), i, GL_TRUE, 0, GL_WRITE_ONLY, GL_R16F);
  //
  //  // Bind opacity volume texture
  //  glActiveTexture(GL_TEXTURE0);
  //  glBindTexture(GL_TEXTURE_3D, tex_supervoxel->GetTextureID());
  //  glUniform1i(glGetUniformLocation(cpsupervoxellevel->GetProgramID(), "TexSuperVoxelVolume"), 0);
  //
  //  // Upload volume dimensions
  //  glUniform3f(
  //    glGetUniformLocation(cpsupervoxellevel->GetProgramID(), "VolumeDimensionsBase")
  //    , (float)wd_b, (float)ht_b, (float)dp_b
  //  );
  //
  //  glUniform3f(
  //    glGetUniformLocation(cpsupervoxellevel->GetProgramID(), "VolumeDimensionsLevel")
  //    , (float)wd_l, (float)ht_l, (float)dp_l
  //  );
  //
  //  // Current mip map level being used
  //  glUniform1f(
  //    glGetUniformLocation(cpsupervoxellevel->GetProgramID(), "MipMapLevel")
  //    , (float)i
  //  );
  //
  //  // Current mip map level being used
  //  glUniform1i(
  //    glGetUniformLocation(cpsupervoxellevel->GetProgramID(), "PositionMultiplier")
  //    , (int)pow(2, i)
  //  );
  //
  //  // x
  //  int disp_w = 2;
  //  while (disp_w < wd_l) disp_w = disp_w * 2;
  //  if (disp_w < 8) disp_w = 8;
  //  int num_groups_x = disp_w / 8;
  //
  //  // y
  //  int disp_h = 2;
  //  while (disp_h < ht_l) disp_h = disp_h * 2;
  //  if (disp_h < 8) disp_h = 8;
  //  int num_groups_y = disp_h / 8;
  //
  //  // z
  //  int disp_d = 2;
  //  while (disp_d < dp_l) disp_d = disp_d * 2;
  //  if (disp_d < 8) disp_d = 8;
  //  int num_groups_z = disp_d / 8;
  //
  //  glActiveTexture(GL_TEXTURE1);
  //  // dispatch for level i
  //  glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
  //}
  //
  //glActiveTexture(GL_TEXTURE0);
  //glBindTexture(GL_TEXTURE_3D, 0);
  //
  //gl::ExitOnGLError("ERROR: After SetData");
  //
  //return tex_supervoxel;
  //
  //double dens_val = data_manager->GetCurrentVolume()->GetMaxDensity();
  //int w = glm::ceil(dens_val);
  //int h = glm::ceil(maximum_standard_deviation);
  //
  //GLfloat* preintegrationvalues = new GLfloat[w * h];
  //for (int iw = 0; iw < w; iw++)
  //{
  //  for (int ih = 0; ih < h; ih++)
  //  {
  //    preintegrationvalues[iw + (ih * w)] = (float)OpacityGaussianEvaluation(iw, ih);
  //  }
  //}
  //
  //glsl_preintegration_lookup = new gl::GLTexture2D(w, h);
  //glsl_preintegration_lookup->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
  //glsl_preintegration_lookup->SetData(preintegrationvalues, GL_R16F, GL_RED, GL_FLOAT);
  //
  //delete[] preintegrationvalues;
  //
  //gl::ExitOnGLError("ERROR: After SetData");

}

