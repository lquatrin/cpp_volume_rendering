#include "utils.h"

#include <cmath>
#include <cstdarg>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/common.hpp>
#include <glm/ext.hpp>


void Print (glm::mat4 mat)
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      std::cout << mat[j][i] << " ";
    }
    std::cout << std::endl;
  }
}

int Clamp (int value, int lower_bound, int higher_bound)
{
  if (value < lower_bound)
    value = lower_bound;
  if (value > higher_bound)
    value = higher_bound;

  return value;
}

float Clamp (float value, float lower_bound, float higher_bound)
{
  if (value < lower_bound)
    value = lower_bound;
  if (value > higher_bound)
    value = higher_bound;

  return value;
}

bool IsNaN (double x)
{
  return x != x;
}

double Cotangent (double angle)
{
  return 1.0 / tan(angle);
}

double DegreesToRadians (double degrees)
{
  return degrees * (glm::pi<double>() / 180.0);
}

double RadiansToDegrees (double radians)
{
  return radians * (180.0 / glm::pi<double>());
}

Matrix4f CreateProjectionMatrix (float fovy, float aspect_ratio, float near_plane, float far_plane)
{
  Matrix4f out = { { 0 } };

  const float
    y_scale = (float)Cotangent(DegreesToRadians((double)fovy / 2.0)),
    x_scale = y_scale / aspect_ratio,
    frustum_length = far_plane - near_plane;

  out.m[0] = x_scale;
  out.m[5] = y_scale;
  out.m[10] = ((far_plane + near_plane) / frustum_length);
  out.m[11] = -1;
  out.m[14] = ((2 * near_plane * far_plane) / frustum_length);

  return out;
}

float Constrain (float x, float a, float b)
{
  if (x < a)
    return a;
  if (x > b)
    return b;
  return x;
}

float Q_rsqrt (float number)
{
  long i;
  float x2, y;
  const float threehalfs = 1.5f;

  x2 = number * 0.5F;
  y = number;
  i = *(long *)&y;                              // evil floating point bit level hacking
  i = 0x5f3759df - (i >> 1);                    // what the fuck?
  y = *(float *)&i;
  y = y * (threehalfs - (x2 * y * y));          // 1st iteration
  //y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

  return y;
}

double Average (int num, ...)
{
  va_list arguments;
  double sum = 0;

  va_start (arguments, num);
  for (int x = 0; x < num; x++) 
    sum += va_arg (arguments, double);
  va_end (arguments);

  return sum / (double)num;
}

double DistanceManhattan(glm::dvec3 a, glm::dvec3 b)
{
  return (glm::abs(a.x - b.x) + glm::abs(a.y - b.y) + glm::abs(a.z - b.z));
}

double Distance(glm::dvec3 a, glm::dvec3 b)
{
  return glm::distance(a, b);
}

double DistanceManhattan (glm::vec3 a, glm::vec3 b)
{
  return (glm::abs(a.x - b.x) + glm::abs(a.y - b.y) + glm::abs(a.z - b.z));
}

double Distance (glm::vec3 a, glm::vec3 b)
{
  return glm::distance(a,b);
}

void TranslateMatrix (Matrix4f matrix, float wx, float hy, float dz)
{

}

glm::vec3 RodriguesRotation (glm::vec3 v, float teta, glm::vec3 k)
{
  glm::vec3 v_rot = v * glm::cos(teta) +
    glm::cross(k, v) * glm::sin(teta) +
    k * glm::dot(k, v) * (1.0f - glm::cos(teta));

  return glm::normalize(v_rot);
}

glm::dvec3 RodriguesRotation (glm::dvec3 v, double teta, glm::dvec3 k)
{
  glm::dvec3 v_rot = v * glm::cos(teta) +
    glm::cross(k, v) * glm::sin(teta) +
    k * glm::dot(k, v) * (1.0f - glm::cos(teta));

  return glm::normalize(v_rot);
}