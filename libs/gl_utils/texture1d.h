/**
 * Texture 1D class
 *
 * About Memory Limitations:
 * CUDA: 
 * https://stackoverflow.com/questions/13401124/1d-texture-memory-limits
 * https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#features-and-technical-specifications
 *
 * OPENGL:
 * https://www.khronos.org/registry/OpenGL/specs/gl/glspec45.core.pdf (225)
 * https://www.khronos.org/opengl/wiki/Texture_Storage
 * https://community.khronos.org/t/texture-buffer-object-how-it-works/53551
 * https://stackoverflow.com/questions/21424968/what-is-the-purpose-of-opengl-texture-buffer-objects
 * https://www.khronos.org/opengl/wiki/Shader_Storage_Buffer_Object
 * https://www.khronos.org/opengl/wiki/Buffer_Texture
 * http://jotschi.de/2009/11/29/opengl-tbos-texture-buffer-objects/
 * https://stackoverflow.com/questions/45317764/opengl-4-5-shader-storage-buffer-objects-layout
**/
#ifndef GL_UTILS_TEXTURE1D_H
#define GL_UTILS_TEXTURE1D_H

#include <gl_utils/utils.h>

namespace gl
{
  class Texture1D
  {
  public:
    Texture1D (unsigned int length);
    ~Texture1D ();

    void GenerateTexture (GLint min_filter_param, GLint max_filter_param
    , GLint wrap_s_param);

    bool SetData (GLvoid* data, GLint internalformat, GLenum format, GLenum type);
    
    GLuint GetTextureID ();

    unsigned int GetLength ();
  private:
    void DestroyTexture ();
    unsigned int m_size;
    unsigned int m_length;
    GLuint m_textureID;
  };
}

#endif