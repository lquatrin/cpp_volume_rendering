#ifndef GL_UTILS_TEXTURE2D_H
#define GL_UTILS_TEXTURE2D_H

#include <gl_utils/utils.h>

namespace gl
{
  class Texture2D
  {
  public:
    Texture2D (unsigned int width, unsigned int height);
    ~Texture2D ();

    void GenerateTexture (GLint min_filter_param, GLint max_filter_param
    , GLint wrap_s_param, GLint wrap_t_param);

    bool SetData(GLvoid* data, GLint internalformat, GLenum format, GLenum type);
    
    GLuint GetTextureID ();

    unsigned int GetWidth ();
    unsigned int GetHeight ();
  private:
    void DestroyTexture ();
    unsigned int m_width;
    unsigned int m_height;
    GLuint m_textureID;
  };
}

#endif