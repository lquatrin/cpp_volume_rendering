#include "timer.h"

namespace gl
{
  Timer::Timer ()
  {
  }
  
  Timer::~Timer ()
  {
  }

  void Timer::Start ()
  {
    glGenQueries(1, &query);
    glBeginQuery(GL_TIME_ELAPSED, query);
  }

  double Timer::End ()
  {
    glEndQuery(GL_TIME_ELAPSED);

    // retrieving the recorded elapsed time
    // wait until the query result is available
    GLint done = 0;
    while (!done) {
      glGetQueryObjectiv(query,
        GL_QUERY_RESULT_AVAILABLE,
        &done);
    }

    // get the query result
    glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
    return elapsed_time / 1000000.0;
  }
}