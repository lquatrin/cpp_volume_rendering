#include "geometry.h"
#include <cmath>

int Orientation (glm::vec2 v1, glm::vec2 v2)
{
  float val = v1.y * v2.x - v1.x * v2.y;
  if (val == 0.0f) return 0;
  return (val > 0.0f) ? 1 : 2;
}

float AreaOfTriangle (float AB_lenght, float BC_lenght, float CA_lenght)
{
  float a = AB_lenght; float b = BC_lenght; float c = CA_lenght;
  float s = (a + b + c) * 0.5f;
  return sqrt (s*(s - a)*(s - b)*(s - c));
}

float AreaOfTriangle (float b, float h)
{
  return 0.5f * b * h;
}

// https://en.wikipedia.org/wiki/Barycentric_coordinate_system
// https://www.iue.tuwien.ac.at/phd/nentchev/node31.html
// https://www.cdsimpson.net/2014/10/barycentric-coordinates.html
glm::vec4 BarycentricCoordinates (Tetrahedron& tet, glm::vec3 p)
{
  static auto ScalarTripleProduct = [](glm::vec3 a, glm::vec3 b, glm::vec3 c)
  {
    return glm::dot(a, glm::cross(b, c));
  };

  glm::vec3 vap = p - tet.a;
  glm::vec3 vbp = p - tet.b;

  glm::vec3 vab = tet.b - tet.a;
  glm::vec3 vac = tet.c - tet.a;
  glm::vec3 vad = tet.d - tet.a;

  glm::vec3 vbc = tet.c - tet.b;
  glm::vec3 vbd = tet.d - tet.b;

  float va6 = ScalarTripleProduct(vbp, vbd, vbc);
  float vb6 = ScalarTripleProduct(vap, vac, vad);
  float vc6 = ScalarTripleProduct(vap, vad, vab);
  float vd6 = ScalarTripleProduct(vap, vab, vac);

  float v6 = 1.0f / ScalarTripleProduct(vab, vac, vad);

  return glm::vec4(va6 * v6, vb6 * v6, vc6 * v6, vd6 * v6);
}