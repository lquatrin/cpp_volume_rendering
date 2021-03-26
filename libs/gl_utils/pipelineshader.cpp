#include "pipelineshader.h"

#include <GL/glew.h>

#include <cstdio>
#include <iostream>
#include <cerrno>

namespace gl
{
char* PipelineShader::TextFileRead (const char* file_name)
{
#ifdef _DEBUG
  printf("File Name TextFileRead \"%s\"\n", file_name);
#endif
  FILE *file_source;
  errno_t err;

  char *content = NULL;
  int count = 0;
  if (file_name != NULL)
  {
    err = fopen_s(&file_source, file_name, "rt");

    if (file_source != NULL)
    {
      fseek(file_source, 0, SEEK_END);
      count = ftell(file_source);
      rewind(file_source);

      if (count > 0) {
        content = (char *)malloc(sizeof(char)* (count + 1));
        count = (int)fread(content, sizeof(char), count, file_source);
        content[count] = '\0';
      }
      fclose(file_source);
    }
    else
    {
      printf("\nFile \"%s\" not found", file_name);
      getchar();
      exit(1);
    }
  }
  return content;
}

void PipelineShader::CompileShader (GLuint shader_id, std::string filename)
{
  char* shader_source = PipelineShader::TextFileRead(filename.c_str());
  const char* const_shader_source = shader_source;

  glShaderSource(shader_id, 1, &const_shader_source, NULL);
  free(shader_source);

  glCompileShader(shader_id);
}

PipelineShader::PipelineShader ()
{
  vec_vertex_shaders_names.clear();
  vec_vertex_shaders_ids.clear();
  vec_fragment_shaders_names.clear();
  vec_fragment_shaders_ids.clear();
  vec_geometry_shaders_names.clear();
  vec_geometry_shaders_ids.clear();
}

bool PipelineShader::AddShaderFile (TYPE type, std::string filename)
{
  if (type == TYPE::VERTEX)
    vec_vertex_shaders_names.push_back(filename);
  else if (type == TYPE::FRAGMENT)
    vec_fragment_shaders_names.push_back(filename);
  else if (type == TYPE::GEOMETRY)
    vec_geometry_shaders_names.push_back(filename);
  gl::ExitOnGLError("GLShader: Unable to attach a new shader file.");
  return true;
}

PipelineShader::PipelineShader (std::string vert, std::string frag)
  : PipelineShader()
{
  m_hasGeometryShader = false;
  if (GLEW_ARB_vertex_shader)
  {
#ifdef _DEBUG
    printf("Ready for GLSL - vertex and fragment units\n");
#endif
  }
  else {
    printf("lqc: Error on GLShader\n");
    exit(1);
  }

  AddShaderFile(TYPE::VERTEX, vert);
  AddShaderFile(TYPE::FRAGMENT, frag);
  LoadAndLink();

  Unbind();
}


PipelineShader::PipelineShader(char *vert, char *frag, char* geom)
  : PipelineShader()
{
  if (geom != NULL)
  {
    m_hasGeometryShader = true;
    if (glewIsSupported("GL_VERSION_4_0") && GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GL_EXT_geometry_shader4)
    {
#ifdef _DEBUG
      printf("Ready for GLSL - vertex, fragment, and geometry units\n");
#endif
    }
    else {
      printf("lqc: Error on GLShader\n");
      exit(1);
    }
  }
  else
  {
    m_hasGeometryShader = false;
    if (GLEW_ARB_vertex_shader)
    {
#ifdef _DEBUG
      printf("Ready for GLSL - vertex and fragment units\n");
#endif
    }
    else {
      printf("lqc: Error on GLShader\n");
      exit(1);
    }
  }

  AddShaderFile(TYPE::VERTEX, vert);
  AddShaderFile(TYPE::FRAGMENT, frag);
  if (m_hasGeometryShader) AddShaderFile(TYPE::GEOMETRY, geom);
  LoadAndLink();

  Unbind();
}

PipelineShader::~PipelineShader()
{
  Clear();
  gl::ExitOnGLError("ERROR: Could not destroy the shaders");
}

bool PipelineShader::LoadAndLink()
{
  if (shader_program == -1)
    shader_program = glCreateProgram();

  for (unsigned int i = 0; i < vec_vertex_shaders_names.size(); i++)
  {
    GLuint new_shader = glCreateShader(GL_VERTEX_SHADER);
    CompileShader(new_shader, vec_vertex_shaders_names[i]);

    vec_vertex_shaders_ids.push_back(new_shader);
    assert(vec_vertex_shaders_ids[i] == new_shader);

    glAttachShader(shader_program, new_shader);
  }

  for (unsigned int i = 0; i < vec_fragment_shaders_names.size(); i++)
  {
    GLuint new_shader = glCreateShader(GL_FRAGMENT_SHADER);
    CompileShader(new_shader, vec_fragment_shaders_names[i]);

    vec_fragment_shaders_ids.push_back(new_shader);
    assert(vec_fragment_shaders_ids[i] == new_shader);

    glAttachShader(shader_program, new_shader);
  }

  for (unsigned int i = 0; i < vec_geometry_shaders_names.size(); i++)
  {
    GLuint new_shader = glCreateShader(GL_GEOMETRY_SHADER);
    CompileShader(new_shader, vec_geometry_shaders_names[i]);

    vec_geometry_shaders_ids.push_back(new_shader);
    assert(vec_geometry_shaders_ids[i] == new_shader);

    glAttachShader(shader_program, new_shader);
  }

  glLinkProgram(shader_program);

  gl::ExitOnGLError("GLShader: Unable to load and link shaders.");
  return true;
}

bool PipelineShader::Reload()
{
  Clear();

  LoadAndLink();
  Bind();

  for (std::map<std::string, UniformVariable>::iterator it = uniform_variables.begin(); it != uniform_variables.end(); ++it)
    uniform_variables[it->first].location = glGetUniformLocation(shader_program, it->first.c_str());
  BindUniforms();

  printf("Shader reloaded with id = %d\n", shader_program);

  return true;
}

void PipelineShader::Clear()
{
  Unbind();

  for (unsigned int i = 0; i < vec_fragment_shaders_ids.size(); i++)
  {
    glDetachShader(shader_program, vec_fragment_shaders_ids[i]);
    glDeleteShader(vec_fragment_shaders_ids[i]);
  }

  for (unsigned int i = 0; i < vec_geometry_shaders_ids.size(); i++)
  {
    glDetachShader(shader_program, vec_geometry_shaders_ids[i]);
    glDeleteShader(vec_geometry_shaders_ids[i]);
  }

  for (unsigned int i = 0; i < vec_vertex_shaders_ids.size(); i++)
  {
    glDetachShader(shader_program, vec_vertex_shaders_ids[i]);
    glDeleteShader(vec_vertex_shaders_ids[i]);
  }

  vec_vertex_shaders_ids.clear();
  vec_fragment_shaders_ids.clear();
  vec_geometry_shaders_ids.clear();
}

void PipelineShader::SetGeometryShaderPrimitives(GLenum in, GLenum out)
{
  m_geometryPrimitiveIn = in;
  m_geometryPrimitiveOut = out;
  int temp;
  glProgramParameteriEXT(shader_program, GL_GEOMETRY_INPUT_TYPE_EXT, in);
  glProgramParameteriEXT(shader_program, GL_GEOMETRY_OUTPUT_TYPE_EXT, out);
  glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &temp);
  glProgramParameteriEXT(shader_program, GL_GEOMETRY_VERTICES_OUT_EXT, temp);
}

void PipelineShader::SetGeometryShaderPrimitives(GS_INPUT in, GS_OUTPUT out)
{
  m_geometryPrimitiveIn = in;
  m_geometryPrimitiveOut = out;
  int temp;
  glProgramParameteriEXT(shader_program, GL_GEOMETRY_INPUT_TYPE_EXT, in);
  glProgramParameteriEXT(shader_program, GL_GEOMETRY_OUTPUT_TYPE_EXT, out);
  glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_EXT, &temp);
  glProgramParameteriEXT(shader_program, GL_GEOMETRY_VERTICES_OUT_EXT, temp);
}

}