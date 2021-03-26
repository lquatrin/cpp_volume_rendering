#ifndef GL_UTILS_GRID_21_H
#define GL_UTILS_GRID_21_H

#include <gl_utils/utils.h>

namespace gl
{
  class Grid_21
  {
  public:
    Grid_21 ();
    ~Grid_21 ();
  
    void Create ();
    void Destroy ();
    void Draw (void);
  private:
    GLuint BufferIds[3];
  };

}

#endif