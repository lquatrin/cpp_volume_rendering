/**
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef VOL_VIS_UTILS_TETRAHEDRON_H
#define VOL_VIS_UTILS_TETRAHEDRON_H

#include <glm/glm.hpp>

namespace vis
{

  class Tetrahedron
  {
  public:
    Tetrahedron () {}
    ~Tetrahedron () {}

    glm::vec4 BarycentricCoordinates (glm::vec3 pt);

    glm::vec3 vert[4];
    glm::vec3 prop[4];
  protected:

  private:
  };
}

#endif