#ifndef GL_UTILS_TEXTURE3D_H
#define GL_UTILS_TEXTURE3D_H

#include <gl_utils/utils.h>

#include <glm/glm.hpp>

namespace gl
{
  class Texture3D
  {
  public:
    Texture3D (unsigned int width, unsigned int height, unsigned int depth);
    Texture3D (glm::ivec3 size);
    ~Texture3D ();

    void GenerateTexture (GLint min_filter_param, GLint max_filter_param
    , GLint wrap_s_param, GLint wrap_t_param, GLint wrap_r_param, bool generatemipmap = false);

    bool SetData (GLvoid* data, GLint internalformat, GLenum format, GLenum type);

    GLuint GetTextureID ();

    unsigned int GetWidth ();
    unsigned int GetHeight ();
    unsigned int GetDepth ();
  
  protected:

  private:
    void DestroyTexture ();
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_depth;
    GLuint m_textureID;
  };
}

#endif