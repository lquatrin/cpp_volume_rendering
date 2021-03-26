#include "arrayobject.h"

namespace gl
{
  void ArrayObject::Unbind ()
  {
    glBindVertexArray(0);
  }

  ArrayObject::ArrayObject (unsigned int number_of_vertex_attribute_locations)
    : m_arrays (0), m_id (0)
  {
    glGenVertexArrays(1, &m_id);
    gl::ExitOnGLError("ERROR: Could not generate the VAO");

    Bind();
    EnableGenericArraysAttribs(number_of_vertex_attribute_locations);
    Unbind();
  }

  ArrayObject::~ArrayObject ()
  {
    glDeleteVertexArrays(1, &m_id);
    gl::ExitOnGLError("ERROR: Could not destroy the vertex array");
  }

  void ArrayObject::Bind ()
  {
    GLint currentvaoid;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentvaoid);
    if (currentvaoid != m_id)
    {
      glBindVertexArray(m_id);
      gl::ExitOnGLError("ERROR: Could not bind the VAO");
    }
  }

  void ArrayObject::DrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
  {
    Bind();
    glDrawElements(mode, count, type, indices);
    gl::ExitOnGLError("lqc: error in GLVAO::DrawElements(...)");
  }

  void ArrayObject::SetVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer)
  {
    if (index < m_arrays)
      glVertexAttribPointer(index, size, type, normalized, stride, pointer);
    gl::ExitOnGLError("ERROR: Could not set VAO attributes");

  }

  void ArrayObject::EnableGenericArraysAttribs (unsigned int number_of_arrays)
  {
    for (m_arrays = 0; m_arrays < number_of_arrays && m_arrays < GL_MAX_VERTEX_ATTRIBS; m_arrays++)
      glEnableVertexAttribArray(m_arrays);
    gl::ExitOnGLError("ERROR: Could not enable vertex attributes");
  }

  void ArrayObject::DisableGenericArraysAttribs ()
  {
    for (int i = 0; i < (int)m_arrays; i++)
      glDisableVertexAttribArray(i);
  }

  GLuint ArrayObject::GetID ()
  {
    return m_id;
  }
}