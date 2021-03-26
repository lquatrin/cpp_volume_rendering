#include "texture3d.h"
#include <cassert>

#include <GL/glew.h>

//http://renderingpipeline.com/2012/03/partially-resident-textures-amd_sparse_texture/
//https://www.opengl.org/registry/specs/ARB/sparse_texture.txt
//http://www.tinysg.de/techGuides/tg4_prt.html

//http://stackoverflow.com/questions/9224300/what-does-gltexstorage-do
//http://www.ecse.rpi.edu/~wrf/wiki/ComputerGraphicsFall2013/SuperBible/SB6/code/src/
//http://holger.dammertz.org/stuff/notes_VirtualTexturing.html
//http://stackoverflow.com/questions/24146011/what-are-the-limits-of-texture-array-size

namespace gl
{
  Texture3D::Texture3D (unsigned int width, unsigned int height, unsigned int depth)
  {
    m_width = width;
    m_height = height;
    m_depth = depth;

    int maxtex3d;
    glGetIntegerv (GL_MAX_3D_TEXTURE_SIZE, &maxtex3d);
    assert (m_width <= maxtex3d || m_height <= maxtex3d || m_depth <= maxtex3d);
    m_textureID = -1;
  }

  Texture3D::Texture3D (glm::ivec3 size)
  {
    m_width  = size.x;
    m_height = size.y;
    m_depth  = size.z;

    int maxtex3d;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxtex3d);
    assert(m_width <= maxtex3d || m_height <= maxtex3d || m_depth <= maxtex3d);
    m_textureID = -1;
  }

  Texture3D::~Texture3D ()
  {
    DestroyTexture();
  }

  void Texture3D::GenerateTexture (GLint min_filter_param, GLint max_filter_param
    , GLint wrap_s_param, GLint wrap_t_param, GLint wrap_r_param, bool generatemipmap)
  {
    if (m_textureID != -1)
      DestroyTexture();
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_3D, m_textureID);

    if (generatemipmap)
    {
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_3D, GL_GENERATE_MIPMAP, GL_TRUE);
    }
    else
    {
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, min_filter_param);
      glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, max_filter_param);
    }

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap_s_param);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap_t_param);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap_r_param);
  }

  bool Texture3D::SetData (GLvoid* data, GLint internalformat, GLenum format, GLenum type)
  {
    if (m_textureID == -1)
      return false;

    gl::ExitOnGLError("gl::Texture3D: Before Texture3D SetData\n");

    // Bind texture
    glBindTexture(GL_TEXTURE_3D, m_textureID);

    // Set Data
    glTexImage3D(GL_TEXTURE_3D, 0, internalformat, m_width, m_height, m_depth, 0, format, type, data);
    #if _DEBUG
      printf("gl::Texture3D: Texture generated with id %d!\n", m_textureID);
    #endif

    // Unbind texture
    glBindTexture(GL_TEXTURE_3D, 0);

    gl::ExitOnGLError("gl::Texture3D: After Texture3D SetData\n");
    assert(glGetError() == GL_NO_ERROR);

    return true;
  }

  GLuint Texture3D::GetTextureID ()
  {
    return m_textureID;
  }

  unsigned int Texture3D::GetWidth ()
  {
    return m_width;
  }

  unsigned int Texture3D::GetHeight ()
  {
    return m_height;
  }

  unsigned int Texture3D::GetDepth ()
  {
    return m_depth;
  }

  void Texture3D::DestroyTexture ()
  {
    GLint temp_texture = m_textureID;
    glDeleteTextures(1, &m_textureID);
#if _DEBUG
    printf("gl::Texture3D: Texture id %d destroyed!\n", temp_texture);
#endif
    m_textureID = -1;
  }
}