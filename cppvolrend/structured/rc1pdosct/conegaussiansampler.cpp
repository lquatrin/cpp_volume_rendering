/**
 * Cone Sampling with Gaussian Integrals .cpp
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#include "conegaussiansampler.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <gl_utils/texture1d.h>

#include <sstream>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <string>
#include <iostream>

#include <math_utils/utils.h>

#ifndef ERROR_EVAL_SEC_HALF_STEP
#define ERROR_EVAL_SEC_HALF_STEP 0.000005
#endif

///////////////////////////////////////////////////////
// public
ConeGaussianSampler::ConeGaussianSampler ()
{
  cone_half_angle = 30.0;
  initial_step = 3.0;
  
  max_gaussian_packing = CONEPACKING::_7;
  
  covered_distance = 100;

  d_sigma = 1.25;// 1.25054245;
  r_sigma = 2.0;

  gaussian_samples_1 = 0;
  gaussian_samples_3 = 0;
  gaussian_samples_7 = 0;

  ui_weight_percentage = 1.0f;
}

ConeGaussianSampler::~ConeGaussianSampler ()
{ }

float ConeGaussianSampler::GetConeHalfAngle ()
{
  return cone_half_angle;
}

void ConeGaussianSampler::SetConeHalfAngle (float angle)
{
  cone_half_angle = glm::clamp(angle, 0.5f, 89.5f);
}

void ConeGaussianSampler::DecreaseConeHalfAngle ()
{
  if (GetConeHalfAngle() > 10.0)
    SetConeHalfAngle(GetConeHalfAngle() - 1.0);
  else
    SetConeHalfAngle(GetConeHalfAngle() - 0.5);
}

void ConeGaussianSampler::IncreaseConeHalfAngle ()
{
  if (GetConeHalfAngle() < 10.0)
    SetConeHalfAngle(GetConeHalfAngle() + 0.5);
  else
    SetConeHalfAngle(GetConeHalfAngle() + 1.0);
}

float ConeGaussianSampler::GetInitialStep ()
{
  return initial_step;
}

void ConeGaussianSampler::SetInitialStep (float istep)
{
  initial_step = istep > 0.0 ? istep : 0.0;
}

ConeGaussianSampler::CONEPACKING ConeGaussianSampler::GetMaxGaussianPacking ()
{
  return max_gaussian_packing;
}

void ConeGaussianSampler::SetMaxGaussianPacking (ConeGaussianSampler::CONEPACKING s_type)
{
  max_gaussian_packing = s_type;
}

int ConeGaussianSampler::GetMaxGaussianPackingInt ()
{
  if (GetMaxGaussianPacking() == CONEPACKING::_1)
    return 0;
  else if (GetMaxGaussianPacking() == CONEPACKING::_3)
    return 1;
  else if (GetMaxGaussianPacking() == CONEPACKING::_7)
    return 2;
  return -1;
}

void ConeGaussianSampler::SetMaxGaussianPacking (int p)
{
  if (p == 0)
    SetMaxGaussianPacking(CONEPACKING::_1);
  else if (p == 1)
    SetMaxGaussianPacking(CONEPACKING::_3);
  else if (p == 2)
    SetMaxGaussianPacking(CONEPACKING::_7);
}

void ConeGaussianSampler::PreviousGaussianPacking ()
{
  if (GetMaxGaussianPacking() == CONEPACKING::_3)
    SetMaxGaussianPacking(CONEPACKING::_1);
  else if (GetMaxGaussianPacking() == CONEPACKING::_7)
    SetMaxGaussianPacking(CONEPACKING::_3);
}

void ConeGaussianSampler::NextGaussianPacking ()
{
  if (GetMaxGaussianPacking() == CONEPACKING::_1)
    SetMaxGaussianPacking(CONEPACKING::_3);
  else if (GetMaxGaussianPacking() == CONEPACKING::_3)
    SetMaxGaussianPacking(CONEPACKING::_7);
}

std::string ConeGaussianSampler::GetCurrentMaxGaussianPackingStr ()
{
  if (GetMaxGaussianPacking() == ConeGaussianSampler::CONEPACKING::_1)  return  "1";
  if (GetMaxGaussianPacking() == ConeGaussianSampler::CONEPACKING::_3)  return  "3";
  if (GetMaxGaussianPacking() == ConeGaussianSampler::CONEPACKING::_7)  return  "7";
}

float ConeGaussianSampler::GetCoveredDistance ()
{
  return covered_distance;
}

void ConeGaussianSampler::SetCoveredDistance (float midist)
{
  covered_distance = midist > 10.0f ? midist : 10.0f;
}

float ConeGaussianSampler::GetIntegrationHalfStepMultiplier ()
{
  return d_sigma;
}

void ConeGaussianSampler::SetIntegrationHalfStepMultiplier (float rdir_sstep_mul)
{
  d_sigma = rdir_sstep_mul;
  d_sigma = glm::clamp(d_sigma, 1.0f, 3.0f);
}

float ConeGaussianSampler::GetGaussianSigmaLimitMultiplier ()
{
  return r_sigma;
}

void ConeGaussianSampler::SetGaussianSigmaLimitMultiplier (float s)
{
  r_sigma = s;
  r_sigma = glm::clamp(r_sigma, 0.5f, 3.0f);
}

int ConeGaussianSampler::GetNumberOfComputedConeSections ()
{
  return data_cone_sectionsinfo.size();
}

gl::Texture1D* ConeGaussianSampler::GetConeSectionsInfoTex ()
{
  gl::Texture1D* tex1d = NULL;

  if (GetNumberOfComputedConeSections() > 0)
  {
    GLfloat* values1d = new GLfloat[GetNumberOfComputedConeSections() * 4];
    for (int i = 0; i < GetNumberOfComputedConeSections(); i++)
    {
      // in this case, it is not the "half step"
      values1d[(i * 4) + 0] = data_cone_intervalsinfo[i].s_distance;
      values1d[(i * 4) + 1] = data_cone_sectionsinfo[i].mip_map_level;
      values1d[(i * 4) + 2] = data_cone_sectionsinfo[i].d_integral;
      values1d[(i * 4) + 3] = data_cone_sectionsinfo[i].amplitude;
    }

    tex1d = new gl::Texture1D(GetNumberOfComputedConeSections());
    tex1d->GenerateTexture(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE);
    tex1d->SetData(values1d, GL_RGBA16F, GL_RGBA, GL_FLOAT);

    delete[] values1d;

    gl::ExitOnGLError("ERROR: After SetData");
  }

  return tex1d;
}

std::vector<ConeGaussianSampler::SectionInfo> ConeGaussianSampler::GetConeSectionsInfoVec ()
{
  return data_cone_sectionsinfo;
}

void ConeGaussianSampler::ComputeConeIntegrationSteps (double min_sg_gaussian)
{
  data_cone_sectionsinfo.clear();
  gaussian_samples_1 = 0;
  gaussian_samples_3 = 0;
  gaussian_samples_7 = 0;

  // 3 axis rays
  ////////////////////////////////////////////////////////////////////////
  {
    double adj_angle = GetConeHalfAngle() / D_HEMISPHERE_CONE_DIV_3;
    double t1 = (GetConeHalfAngle() - adj_angle) * glm::pi<double>() / 180.0;

    ray3_axis[0] = RodriguesRotation(glm::vec3(0, 0, 1), t1, glm::vec3(0, 1, 0));
    ray3_axis[0] = RodriguesRotation(ray3_axis[0],
      30.0 * glm::pi<double>() / 180.0,
      glm::vec3(0, 0, 1));

    double angle_t = 120.0 * glm::pi<double>() / 180.0;
    ray3_axis[1] = RodriguesRotation(ray3_axis[0], angle_t, glm::vec3(0, 0, 1));
    ray3_axis[2] = RodriguesRotation(ray3_axis[1], angle_t, glm::vec3(0, 0, 1));
  }
  ////////////////////////////////////////////////////////////////////////

  // 7 axis rays
  ////////////////////////////////////////////////////////////////////////
  {
    ray7_axis[0] = glm::vec3(0, 0, 1);

    double adj_angle = GetConeHalfAngle() / D_HEMISPHERE_CONE_DIV_7;
    double t1 = (GetConeHalfAngle() - adj_angle) * glm::pi<double>() / 180.0;

    ray7_axis[1] = RodriguesRotation(ray7_axis[0], t1, glm::vec3(0, 1, 0));

    double angle_t = 60.0 * glm::pi<double>() / 180.0;
    ray7_axis[2] = RodriguesRotation(ray7_axis[1], angle_t, glm::vec3(0, 0, 1));
    ray7_axis[3] = RodriguesRotation(ray7_axis[2], angle_t, glm::vec3(0, 0, 1));
    ray7_axis[4] = RodriguesRotation(ray7_axis[3], angle_t, glm::vec3(0, 0, 1));
    ray7_axis[5] = RodriguesRotation(ray7_axis[4], angle_t, glm::vec3(0, 0, 1));
    ray7_axis[6] = RodriguesRotation(ray7_axis[5], angle_t, glm::vec3(0, 0, 1));
  }
  ////////////////////////////////////////////////////////////////////////

  data_cone_intervalsinfo.clear();
  int n_gaussians = 1;

  double curr_pos = initial_step;
  double sigma_gaussian = min_sg_gaussian;

  // For each section separator...
  while (!AddGaussianSampleStep(curr_pos, sigma_gaussian, &n_gaussians))
    sigma_gaussian *= 2.0;

  while (curr_pos < GetCoveredDistance())
  {
    // first half of the interval
    double si = (GetIntegrationHalfStepMultiplier() * sigma_gaussian);

    // For each section separator...
    while (!AddGaussianSampleStep(curr_pos + si + (GetIntegrationHalfStepMultiplier() * sigma_gaussian), sigma_gaussian, &n_gaussians))
      sigma_gaussian *= 2.0;

    // second half of the interval
    si += (GetIntegrationHalfStepMultiplier() * sigma_gaussian);

    // add this interval
    data_cone_intervalsinfo.push_back(IntervalsInfo(curr_pos, si));

    // update current position
    curr_pos += si;
  }
  
  ComputeAdditionalInfo(min_sg_gaussian);
}

double ConeGaussianSampler::GetRay3AdjacentWeight ()
{
  return ray3_adj_weight;
}

double ConeGaussianSampler::GetRay7AdjacentWeight ()
{
  return ray7_adj_weight;
}

glm::vec3 ConeGaussianSampler::Get3ConeRayID (int i)
{
  if (i < 0 || i > 2)
    return glm::vec3(0);
  return ray3_axis[i];
}

glm::vec3 ConeGaussianSampler::Get7ConeRayID (int i)
{
  if (i < 0 || i > 6)
    return glm::vec3(0);
  return ray7_axis[i];
}

void ConeGaussianSampler::SetUIWeightPercentage (float a)
{
  ui_weight_percentage = a;
}

///////////////////////////////////////////////////////
// private
void ConeGaussianSampler::ComputeAdditionalInfo(double min_sg_gaussian)
{
  // Ray adjacent weights
  ray3_adj_weight    = glm::dot(glm::vec3(0, 0, 1), ray3_axis[0]);
  ray7_adj_weight    = glm::dot(glm::vec3(0, 0, 1), ray7_axis[1]);

  // Compute the rescale factor:
  //
  // \    |
  //  \---|---/
  //   \  |  / cut based on cone_radius
  //    \-|-/
  //     \|/
  //
  // In the end, multiply by the 1D Gaussian Integral
  //

  for (int i = 0; i < data_cone_sectionsinfo.size() - 1; i++)
  {
    if (!(data_cone_sectionsinfo[i].number_of_gaussians <= data_cone_sectionsinfo[i + 1].number_of_gaussians))
    {
      std::cout << "Wrong number of gaussians per section" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  double SumS = 0.0;
  if (!(data_cone_sectionsinfo.size() == data_cone_intervalsinfo.size() + 1))
  {
    std::cout << "Wrong number of sections and intervals" << std::endl;
    exit(EXIT_FAILURE);
  }

  // add one last interval with 0 distance to be same size with gaussian samples
  double last_pos = data_cone_intervalsinfo[data_cone_intervalsinfo.size() - 1].s_position + data_cone_intervalsinfo[data_cone_intervalsinfo.size() - 1].s_distance;
  data_cone_intervalsinfo.push_back(IntervalsInfo(last_pos, 0));

  for (int i = 0; i < data_cone_sectionsinfo.size(); i++)
  {
    SectionInfo* si = &data_cone_sectionsinfo[i];

    if      (si->number_of_gaussians == 1)  gaussian_samples_1  += 1;
    else if (si->number_of_gaussians == 3)  gaussian_samples_3  += 1;
    else if (si->number_of_gaussians == 7)  gaussian_samples_7  += 1;

    double cone_radius_t = si->cone_radius;

    // define the distance to be integrated
    if (i == 0)
      si->d_integral = si->sampled_gaussian_sigma * glm::sqrt(2.0 * glm::pi<double>()) * 0.5;
    else
      si->d_integral = data_cone_intervalsinfo[i - 1].s_distance * 0.5;

    // The integration step is the 3D gaussian...
    double pr = IntegrateGaussian(si->sampled_gaussian_sigma, cone_radius_t);

    // ... divided by the cone radius (circle area)
    double Ac = glm::pi<double>() * cone_radius_t * cone_radius_t;

    // gaussian integral based on sampled sigma
    double Ig = si->sampled_gaussian_sigma * glm::sqrt(2.0 * glm::pi<double>());

    si->amplitude = ((pr*pr) * (Ig*Ig)) / Ac;

    //std::cout << i << ": " << si->d_integral << " " << si->amplitude << std::endl;

    si->mip_map_level = glm::log2(si->sampled_gaussian_sigma / min_sg_gaussian);
  }
}

double ConeGaussianSampler::GaussianEval (double x, double sig)
{
  return (1.0 / (glm::sqrt(2.0 * glm::pi<double>()) * sig)) * glm::exp(-(x*x) / (2.0*sig*sig));
}

double ConeGaussianSampler::IntegrateGaussian (double sdev_gaussian, double cone_radius)
{
  // number of segments
  double t = (2.0 * cone_radius) / 0.05;

  // number of segments integer
  int nt = glm::ceil(t);

  // update the segment size
  double segment = (2.0 * cone_radius) / double(nt);
  
  // at the initial integration step, we take 
  //   the medium point to increase accuracy.
  double s0 = -cone_radius + segment * 0.5;

  // Sum all values multiplied by the segment lenght
  double S = 0.0;
  for (int i = 0; i < nt; i++)
    S += GaussianEval(s0 + segment * double(i), sdev_gaussian) * segment;

  return S;
}

///////////////////////////////////////////////////////
// private
bool ConeGaussianSampler::AddGaussianSampleStep (double curr_pos, double sg_gaussian, int *n_gaussians)
{
  if (*n_gaussians > 1)
    return AddGaussianSampleStepWith3(curr_pos, sg_gaussian, n_gaussians);
  (*n_gaussians) = 1;

  // based on the number of cones, we must divide the current angle to evaluate N cones
  double rad_cone_half_angle = GetConeHalfAngle() * glm::pi<double>() / 180.0;

  // compute the current value of cone_radius
  double cone_radius = curr_pos * glm::tan(rad_cone_half_angle);

  // if cone_radius is higher than the threshold
  if (cone_radius > GetGaussianSigmaLimitMultiplier() * sg_gaussian)
  {
    // try to add more gaussians
    if (GetMaxGaussianPacking() > ConeGaussianSampler::CONEPACKING::_1)
      return AddGaussianSampleStepWith3(curr_pos, sg_gaussian, n_gaussians);

    // if we can't, return false to increase the gaussian size
    return false;
  }

  // in this case, sec_half_step is not computed
  data_cone_sectionsinfo.push_back(SectionInfo(1, curr_pos, -1, cone_radius, sg_gaussian));

  return true;
}

bool ConeGaussianSampler::AddGaussianSampleStepWith3 (double curr_pos, double sg_gaussian, int *n_gaussians)
{
  if (*n_gaussians > 3)
    return AddGaussianSampleStepWith7(curr_pos, sg_gaussian, n_gaussians);
  (*n_gaussians) = 3;

  // based on the number of cones, we must divide the current angle to evaluate N cones
  double rad_cone_half_angle = (GetConeHalfAngle() / D_HEMISPHERE_CONE_DIV_3) * glm::pi<double>() / 180.0;
  
  // compute the current value of cone_radius
  double cone_radius = curr_pos * glm::tan(rad_cone_half_angle);
  
  if (cone_radius > GetGaussianSigmaLimitMultiplier() * sg_gaussian)
  {
    // try to add more gaussians
    if (GetMaxGaussianPacking() > ConeGaussianSampler::CONEPACKING::_3)
      return AddGaussianSampleStepWith7(curr_pos, sg_gaussian, n_gaussians);

    // if we can't, return false to increase the gaussian size
    return false;
  }

  // in this case, sec_half_step is not computed
  data_cone_sectionsinfo.push_back(SectionInfo(3, curr_pos, -1, cone_radius, sg_gaussian));

  return true;
}

bool ConeGaussianSampler::AddGaussianSampleStepWith7 (double curr_pos, double sg_gaussian, int *n_gaussians)
{
  // We might try to use more than 7 gaussians per section...
  //if (*n_gaussians > 7)
  //  AddGaussianSampleStepWithXX(curr_pos, sg_gaussian, n_gaussians);
  (*n_gaussians) = 7;

  // based on the number of cones, we must divide the current angle to evaluate N cones
  double rad_cone_half_angle = (GetConeHalfAngle() / D_HEMISPHERE_CONE_DIV_7) * glm::pi<double>() / 180.0;

  // compute the current value of cone_radius
  double cone_radius = curr_pos * glm::tan(rad_cone_half_angle);

  if (cone_radius > GetGaussianSigmaLimitMultiplier() * sg_gaussian)
  {
    // if we can't, return false to increase the gaussian size
    return false;
  }

  // in this case, sec_half_step is not computed
  data_cone_sectionsinfo.push_back(SectionInfo(7, curr_pos, -1, cone_radius, sg_gaussian));

  return true;
}