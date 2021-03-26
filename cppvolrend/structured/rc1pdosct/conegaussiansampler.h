/**
 * Cone Sampling with Gaussian Integrals .h
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
 ****************************************************
 * Cone directions using circle packing in a circle:
 * . https://en.wikipedia.org/wiki/Circle_packing_in_a_circle
 **/
#ifndef CONE_DIRECTIONAL_GAUSSIAN_SAMPLER_H
#define CONE_DIRECTIONAL_GAUSSIAN_SAMPLER_H

#include <glm/glm.hpp>
#include <gl_utils/texture1d.h>

#include <vector>

#define D_HEMISPHERE_CONE_DIV_3  (1.0 + (2.0 / glm::sqrt(3.0))) // 2.158455
#define D_HEMISPHERE_CONE_DIV_7  3.010000                       // 3.010000

class ConeGaussianSampler
{
public:
  typedef struct IntervalsInfo {
    IntervalsInfo (double position, double distance)
    {
      s_position = position;
      s_distance = distance;
    }

    double s_position;
    double s_distance;

  } IntervalsInfo;

  // info for each section
  typedef struct SectionInfo {
    SectionInfo (int _number_of_gaussians,
                 double _distance_from_origin,
                 double _sec_half_step,
                 double _cone_radius,
                 double _sampled_gaussian_sigma)
    {
      number_of_gaussians = _number_of_gaussians;

      distance_from_origin = _distance_from_origin;
      sec_half_step = _sec_half_step;
      cone_radius = _cone_radius;

      sampled_gaussian_sigma = _sampled_gaussian_sigma;
    }

    int number_of_gaussians;

    double distance_from_origin;
    double sec_half_step;
    double cone_radius;

    double sampled_gaussian_sigma;

    double coefficient_cut_radius;
    double coefficient_rescale;
    double d_integral;
    double step_integral;
    double mip_map_level;

    double amplitude;
  } SectionInfo;

  enum CONEPACKING {
    _1 = 0,
    _3 = 1,
    _7 = 2,
  };

  int gaussian_samples_1;
  int gaussian_samples_3;
  int gaussian_samples_7;

public:
  ConeGaussianSampler ();
  ~ConeGaussianSampler ();

  // \ angle |       /
  //  \------|      /
  //   \     |     /
  //    \    |    /
  //     \   |   /
  //      \  |  /
  //       \ | /
  //        \|/
  //         *
  float GetConeHalfAngle ();
  void SetConeHalfAngle (float angle);
  void DecreaseConeHalfAngle ();
  void IncreaseConeHalfAngle ();

  float GetInitialStep ();
  void SetInitialStep (float istep);

  CONEPACKING GetMaxGaussianPacking ();
  void SetMaxGaussianPacking (CONEPACKING gpk);
  int GetMaxGaussianPackingInt ();
  void SetMaxGaussianPacking (int p);
  void PreviousGaussianPacking ();
  void NextGaussianPacking ();
  std::string GetCurrentMaxGaussianPackingStr ();

  float GetCoveredDistance ();
  void SetCoveredDistance (float midist);

  float GetIntegrationHalfStepMultiplier ();
  void SetIntegrationHalfStepMultiplier (float rdir_sstep_mul);

  float GetGaussianSigmaLimitMultiplier ();
  void SetGaussianSigmaLimitMultiplier (float s);

  int GetNumberOfComputedConeSections ();
  gl::Texture1D* GetConeSectionsInfoTex ();
  std::vector<SectionInfo> GetConeSectionsInfoVec ();

  void ComputeConeIntegrationSteps (double min_sg_gaussian);

  double GetRay3AdjacentWeight ();
  double GetRay7AdjacentWeight ();

  glm::vec3 Get3ConeRayID (int i);
  glm::vec3 Get7ConeRayID (int i);
  
  void SetUIWeightPercentage (float a);

  bool cut_on_ray_direction;

  glm::vec3 ray3_axis[3];
  double ray3_adj_weight;
  
  glm::vec3 ray7_axis[7];
  double ray7_adj_weight;

  float cone_half_angle;
  float initial_step;
  CONEPACKING max_gaussian_packing;
  float covered_distance;

  float ui_weight_percentage;

  float d_sigma;
  float r_sigma;

private:
  std::vector<SectionInfo> data_cone_sectionsinfo;
  std::vector<IntervalsInfo> data_cone_intervalsinfo;

private:
  void ComputeAdditionalInfo (double min_sg_gaussian);

  double GaussianEval (double x, double sig);
  double IntegrateGaussian (double min_sg_gaussian, double cone_radius);

  bool AddGaussianSampleStep      (double curr_pos, double sg_gaussian, int *n_gaussians);
  bool AddGaussianSampleStepWith3 (double curr_pos, double sg_gaussian, int *n_gaussians);
  bool AddGaussianSampleStepWith7 (double curr_pos, double sg_gaussian, int *n_gaussians);
};

#endif