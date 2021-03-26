#include "gfilter.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

double GaussianFilter (double sig, double x, bool normalize)
{
  double gf = glm::exp(-(x*x) / (2.0 * sig * sig));
  return (normalize ? 1.0 / (glm::sqrt(2.0 * glm::pi<double>()) * sig) : 1.0) * gf;
}

double GaussianFilter (double sig, double x, double y, bool normalize)
{
  double sig2 = sig * sig;
  double gf = glm::exp(-(x*x + y*y) / (2.0 * sig2));
  return (normalize ? 1.0 / (2.0 * glm::pi<double>() * sig2) : 1.0) * gf;
}

double GaussianFilter (double sig, double x, double y, double z, bool normalize)
{
  double gf = glm::exp(-(x*x + y*y + z*z) / (2.0 * sig * sig));
  return (normalize ? 1.0 / glm::pow(glm::sqrt(2.0 * glm::pi<double>()) * sig, 3.0) : 1.0) * gf;
}