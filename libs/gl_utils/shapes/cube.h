#ifndef GL_UTILS_CUBE_H
#define GL_UTILS_CUBE_H

#include <gl_utils/utils.h>

namespace gl
{
  class Cube
  {
  public:
    Cube ();
    ~Cube ();
  
    void Create ();
    void Destroy ();
    void Draw (void);
  private:
    GLuint BufferIds[3];
  };

}

#endif