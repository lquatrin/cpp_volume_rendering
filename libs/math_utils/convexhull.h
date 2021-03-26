#ifndef MATH_UTILS_CONVEX_HULL_H
#define MATH_UTILS_CONVEX_HULL_H

#include "geometry.h"
#include <glm/glm.hpp>

#include <vector>
/**
* Order of output vertices: Counter Clockwise
*
*     8                       7
*      \                     /
*       *------------------*
*       | *    *    *     *  \
*       |    *  *  *    *  *  \
*   9 - *  *    *    *  *   *  * - 6
*      / *    *    *    *    * |
* 1 - *  *    *  *  *  *   *   * - 5
*     |    *        *    *    /
*     | *     *   *  * *  *  /
* 2 - *  *      *     *     /
*      \    *       *    * /
*       \*     *  *   *   /
*        \ *  *    *     /
*         *-------------*
*        /                \
*       3                  4
*
* http://en.wikipedia.org/wiki/Convex_hull_algorithms
**/

class ConvexHull2D
{
public:
  ConvexHull2D ();
  ~ConvexHull2D ();

  std::vector<glm::vec2> GetCCWConvexPoints();

  virtual void Do (std::vector<glm::vec2> points) = 0;
protected:
  std::vector<glm::vec2> m_ccw_convex_points;
};

namespace ch2d
{
  class Incremental : ConvexHull2D
  {
  public:
    Incremental ();
    ~Incremental ();

    virtual void Do(std::vector<glm::vec2> points);
  };
}

#endif

