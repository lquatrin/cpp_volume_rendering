/**
 * Class time query.
 *
 * https://stackoverflow.com/questions/28175631/how-to-measure-time-performance-of-a-compute-shader
 * http://www.lighthouse3d.com/tutorials/opengl-timer-query/
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef GL_UTILS_TIMER_H
#define GL_UTILS_TIMER_H

#include <GL/glew.h>

namespace gl
{
  class Timer
  {
  public:
    Timer ();
    ~Timer ();

    void Start ();
    // return in miliseconds
    double End ();

  protected:
  private:
    GLuint query;
    GLuint64 elapsed_time;
  };
}

#endif