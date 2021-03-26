/**
 * Our implementation of 2016 paper:
 * . Parallel Distributed, GPU-Accelerated, Advanced Lighting 
 *   Calculations for Large-Scale Volume Visualization
 *
 * Obs: In the paper, the authors consider the aperture angle of the whole angle.
 *      We consider as the "half angle" aperture.
 *
 *  \    |    /
 *   \---|---/ cone apex angle by the authors
 *    \  |  / 
 *     \-| / cone angle considering half angle (our approach)
 *      \|/
 *
 * Considering the "half" angle as the cone aperture is more common for research papers.
**/
#ifndef VOXEL_CONE_TRACING_GAUSSIAN_PRE_PROCESSING
#define VOXEL_CONE_TRACING_GAUSSIAN_PRE_PROCESSING

#include <volvis_utils/reader.h>
#include <volvis_utils/utils.h>
#include <volvis_utils/structuredgridvolume.h>

#include <gl_utils/arrayobject.h>
#include <gl_utils/bufferobject.h>
#include <gl_utils/texture1d.h>
#include <gl_utils/texture3d.h>

#include <gl_utils/pipelineshader.h>

#include "../../volrenderbase.h"

class GBuffer;

class VCTPreProcessing
{
public:
  VCTPreProcessing();
  virtual ~VCTPreProcessing();

  void Destroy()
  {
    if (glsl_supervoxel_meanstddev != nullptr)
      delete glsl_supervoxel_meanstddev;
    glsl_supervoxel_meanstddev = nullptr;

    if (glsl_preintegration_lookup != nullptr)
      delete glsl_preintegration_lookup;
    glsl_preintegration_lookup = nullptr;

    for (int i = 0; i < tree_spr_voxel.size(); i++)
      delete tree_spr_voxel[i];
    tree_spr_voxel.clear();
  }

  double GetMeanFromSuperVoxel (int lvl, int lw, int lh, int ld, int vw, int vh, int vd);
  double GetStdDevFromSuperVoxel (int lvl, int lw, int lh, int ld, int vw, int vh, int vd);

  void PreProcessSuperVoxels (vis::StructuredGridVolume* vol);

  double GaussianEvaluation (double x, double mean, double stddev);
  double OpacityGaussianEvaluation (double mean, double stddev, vis::StructuredGridVolume* vol, vis::TransferFunction* tf);

  void PreProcessPreIntegrationTable (vis::StructuredGridVolume* vol, vis::TransferFunction* tf);

  bool use_glsl_to_precompute_data;
  gl::Texture3D* glsl_supervoxel_meanstddev;
  gl::Texture2D* glsl_preintegration_lookup;

  double maximum_standard_deviation;

  class SuperVoxelLevel
  {
  public:
    typedef struct SuperVoxel {
      double mean;
      double stdv;
    } SuperVoxel;

    SuperVoxelLevel (glm::ivec3 voldim)
    {
      dim = voldim;
      sv_data = new SuperVoxel[dim.x * dim.y * dim.z];
    }

    ~SuperVoxelLevel ()
    {
      delete[] sv_data;
    }

    SuperVoxel* sv_data;
    glm::ivec3 dim;
  protected:

  private:

  };
  std::vector<SuperVoxelLevel*> tree_spr_voxel;

protected:

private:
  gl::Texture3D* GLSLPreComputeSuperVoxels();
  gl::Texture2D* GLSLPreComputePreIntegrationTable();
};

#endif
