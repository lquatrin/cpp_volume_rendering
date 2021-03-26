#include "texture2d.h"

#include <cassert>

#include <GL/glew.h>

namespace gl
{
  Texture2D::Texture2D (unsigned int width, unsigned int height)
  {
    m_width = width;
    m_height = height;

    m_textureID = -1;
  }

  Texture2D::~Texture2D ()
  {
    glDeleteTextures(1, &m_textureID);
  }

  void Texture2D::GenerateTexture (GLint min_filter_param, GLint max_filter_param, GLint wrap_s_param, GLint wrap_t_param)
  {
    if (m_textureID != -1)
      DestroyTexture();
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter_param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, max_filter_param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s_param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t_param);
  }

  bool Texture2D::SetData (GLvoid* data, GLint internalformat, GLenum format, GLenum type)
  {
    if (m_textureID == -1)
      return false;

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    // Set Data
    // For bigger textures: GL_PROXY_TEXTURE_2D
    glTexImage2D(GL_TEXTURE_2D, 0, internalformat, m_width, m_height, 0, format, type, data);
    #if _DEBUG
        printf("texture2d.cpp: Texture generated with id %d!\n", m_textureID);
    #endif

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    
    assert(glGetError() == GL_NO_ERROR);
  
    return true;
  }

  GLuint Texture2D::GetTextureID ()
  {
    return m_textureID;
  }


  unsigned int Texture2D::GetWidth ()
  {
    return m_width;
  }

  unsigned int Texture2D::GetHeight ()
  {
    return m_height;
  }

  void Texture2D::DestroyTexture ()
  {
    GLint temp_texture = m_textureID;
    glDeleteTextures(1, &m_textureID);
    printf("lqc: Texture2D with id %d destroyed!\n", temp_texture);
    m_textureID = -1;
  }
}