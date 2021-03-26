#include "texture1d.h"
#include <GL/glew.h>
#include <cassert>

namespace gl
{
  Texture1D::Texture1D (unsigned int length)
  {
    m_length = length;

    m_textureID = -1;
  }

  Texture1D::~Texture1D ()
  {
    glDeleteTextures(1, &m_textureID);
  }

  void Texture1D::GenerateTexture (GLint min_filter_param, GLint max_filter_param, GLint wrap_s_param)
  {
    if (m_textureID != -1)
      DestroyTexture();
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_1D, m_textureID);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, min_filter_param);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, max_filter_param);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, wrap_s_param);
  }

  bool Texture1D::SetData (GLvoid* data, GLint internalformat, GLenum format, GLenum type)
  {
    if (m_textureID == -1)
      return false;

    // Bind texture
    glBindTexture(GL_TEXTURE_1D, m_textureID);

    // Set Data
    glTexImage1D(GL_TEXTURE_1D, 0, internalformat, m_length, 0, format, type, data);
    #if _DEBUG
      printf("texture1d.cpp: Texture generated with id %d!\n", m_textureID);
    #endif

    // Unbind texture
    glBindTexture(GL_TEXTURE_1D, 0);

    assert(glGetError() == GL_NO_ERROR);

    return true;
  }

  GLuint Texture1D::GetTextureID ()
  {
    return m_textureID;
  }

  unsigned int Texture1D::GetLength ()
  {
    return m_length;
  }

  void Texture1D::DestroyTexture ()
  {
    GLint temp_texture = m_textureID;
    glDeleteTextures(1, &m_textureID);
    #if _DEBUG
    printf("lqc: Texture1D with id %d destroyed!\n", temp_texture);
    #endif
    m_textureID = -1;
  }
}