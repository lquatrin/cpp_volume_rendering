#include "bufferobject.h"

namespace gl
{
  void BufferObject::Unbind (GLenum target)
  {
    glBindBuffer(target, 0);
  }

  BufferObject::BufferObject (GLenum target)
    : m_id (0), m_target (target)
  {
    glGenBuffers(1, &m_id);
    gl::ExitOnGLError("ERROR: Could not generate the Buffer Object");
  }

  BufferObject::~BufferObject ()
  {
    glDeleteBuffers(1, &m_id);
    gl::ExitOnGLError("ERROR: Could not destroy the buffer object");
  }

  void BufferObject::Bind ()
  {
    glBindBuffer(m_target, m_id);
    gl::ExitOnGLError("ERROR: Could not bind the Buffer Object");
  }

  void BufferObject::Unbind ()
  {
    glBindBuffer(m_target, 0);
  }

  void BufferObject::SetBufferData (GLsizeiptr size, const GLvoid *data, GLenum usage)
  {
    Bind();
    glBufferData(m_target, size, data, usage);
    gl::ExitOnGLError("ERROR: Could not set Buffer Object data");
  }

  GLuint BufferObject::GetID ()
  {
    return m_id;
  }
}