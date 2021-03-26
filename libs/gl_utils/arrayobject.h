#ifndef GL_UTILS_ARRAY_OBJECT_H
#define GL_UTILS_ARRAY_OBJECT_H

#include "utils.h"

#include <GL/glew.h>

namespace gl
{
  class ArrayObject
  {
  public:
    static void Unbind ();

    ArrayObject (unsigned int number_of_vertex_attribute_locations = 0);
    ~ArrayObject ();

    void Bind ();

    void DrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices = (GLvoid*)0);

    void SetVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer);

    void EnableGenericArraysAttribs (unsigned int number_of_arrays);
    void DisableGenericArraysAttribs ();

    GLuint GetID ();

  private:
    GLuint m_id;
    unsigned int m_arrays;
  };
}

#endif