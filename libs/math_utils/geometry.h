#ifndef MATH_UTILS_GEOMETRY_H
#define MATH_UTILS_GEOMETRY_H

#include <glm/glm.hpp>

class Tetrahedron
{
public:
  Tetrahedron () 
  {
    a = b = c = d = glm::vec3(0.0f);
  }

  Tetrahedron (glm::vec3 _a, glm::vec3 _b, glm::vec3 _c, glm::vec3 _d)
  {
    a = _a;
    b = _b;
    c = _c;
    d = _d;
  }

  ~Tetrahedron ()
  {
  }

  glm::vec3 a, b, c, d;
protected:
private:
};

/**
* 0 -> Colinear
* 1 -> ClockWise
* 2 -> CounterClockwise
*
*         ^ v2
*        /
*       /
*      /
*     / <---
*    /      |  Orientation (v1 to v2)
*   /       |
*   -----------------> v1
*
*/
int Orientation (glm::vec2 v1, glm::vec2 v2);

float AreaOfTriangle (float AB_lenght, float BC_lenght, float CA_lenght);
float AreaOfTriangle (float b, float h);

glm::vec4 BarycentricCoordinates (Tetrahedron& tet, glm::vec3 p);

#endif