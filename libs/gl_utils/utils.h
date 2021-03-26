#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <ctime>
#include <vector>
#include <string>

#include <GL/glew.h>

#include <glm/glm.hpp>

namespace gl
{
  typedef struct Vertex
  {
	  float Position[4];
	  float Color[4];
  } Vertex;

  void ExitOnGLError (const char* error_message);
  void ShaderInfoLog (GLuint obj);
  void ProgramInfoLog (GLuint obj);
  int  OglError(char *file, int line);

  GLuint LoadShader (const char* file_name, GLenum shader_type);
  char* TextFileRead (const char* file_name);

  glm::ivec3 ComputeShaderGetNumberOfGroups (int w, int h, int d);

  void Vertex3f (glm::vec3 v);
  void Rotatef (float angle, glm::vec3 v);
  void Color3f (glm::vec3 color);
  void Translatef (glm::vec3 t);

}

#endif