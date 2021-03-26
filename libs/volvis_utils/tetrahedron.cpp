#include "tetrahedron.h"

namespace vis
{
  glm::vec4 Tetrahedron::BarycentricCoordinates (glm::vec3 pt)
  {
    static auto ScalarTripleProduct = [](glm::vec3 a, glm::vec3 b, glm::vec3 c) 
    {
      return glm::dot(a, glm::cross(b, c));
    };

    glm::vec3 vap = pt - vert[0];
    glm::vec3 vbp = pt - vert[1];

    glm::vec3 vab = vert[1] - vert[0];
    glm::vec3 vac = vert[2] - vert[0];
    glm::vec3 vad = vert[3] - vert[0];

    glm::vec3 vbc = vert[2] - vert[1];
    glm::vec3 vbd = vert[3] - vert[1];

    float va6 = ScalarTripleProduct(vbp, vbd, vbc);
    float vb6 = ScalarTripleProduct(vap, vac, vad);
    float vc6 = ScalarTripleProduct(vap, vad, vab);
    float vd6 = ScalarTripleProduct(vap, vab, vac);

    float v6 = 1.0f / ScalarTripleProduct(vab, vac, vad);

    return glm::vec4(va6 * v6, vb6 * v6, vc6 * v6, vd6 * v6);
  }
}