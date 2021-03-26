#include "computeshader.h"
#include <gl_utils/utils.h>

namespace gl
{
  ComputeShader::ComputeShader ()
  {
    num_groups_x = 0;
    num_groups_y = 0;
    num_groups_z = 0;

    vec_compute_shader_names.clear();
    vec_compute_shader_ids.clear();
  }

  ComputeShader::~ComputeShader ()
  {
    Unbind();

    for (int i = 0; i < vec_compute_shader_ids.size(); i++)
    {
      glDetachShader(shader_program, vec_compute_shader_ids[i]);
      glDeleteShader(vec_compute_shader_ids[i]);
    }

    vec_compute_shader_names.clear();
    vec_compute_shader_ids.clear();
  }

  bool ComputeShader::LoadAndLink ()
  {
    // Creates a compute shader and the respective program that contains the shader.
    if (shader_program == -1)
      shader_program = glCreateProgram();

    for (int i = 0; i < vec_compute_shader_names.size(); i++)
    {
      GLuint new_shader = glCreateShader(GL_COMPUTE_SHADER);
      
      CompileShader(new_shader, vec_compute_shader_names[i]);
      
      vec_compute_shader_ids.push_back(new_shader);
      assert(vec_compute_shader_ids[i] == new_shader);
    
      glAttachShader(shader_program, new_shader);
    }
    
    glLinkProgram(shader_program);
    glValidateProgram(shader_program);

    gl::ExitOnGLError("gl::ComputeShader >> Unable to link shaders.");

    // Check link status
    int rvalue;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &rvalue);
    if (!rvalue) {
      fprintf(stderr, "Error in linking compute shader program\n");
      GLchar log[10240];
      GLsizei length;
      glGetProgramInfoLog(shader_program, 10239, &length, log);
      fprintf(stderr, "Linker log:\n%s\n", log);
      exit(41);
    }

    gl::ExitOnGLError("gl::ComputeShader >> Unable to load and link shaders.");
    
    return true;
  }

  bool ComputeShader::Reload ()
  {
    Unbind();

    for (int i = 0; i < vec_compute_shader_ids.size(); i++)
    {
      glDetachShader(shader_program, vec_compute_shader_ids[i]);
      glDeleteShader(vec_compute_shader_ids[i]);
    }
    vec_compute_shader_ids.clear();

    LoadAndLink();

    Bind();

    for (std::map<std::string, UniformVariable>::iterator it = uniform_variables.begin(); it != uniform_variables.end(); ++it)
      uniform_variables[it->first].location = glGetUniformLocation(shader_program, it->first.c_str());
    BindUniforms();

    gl::Shader::Unbind();

    return true;
  }

  void ComputeShader::SetShaderFile (std::string filename)
  {
    vec_compute_shader_names.clear();
    vec_compute_shader_names.push_back(filename);
  }

  void ComputeShader::AddShaderFile (std::string filepath)
  {
    vec_compute_shader_names.push_back(filepath);
  }

  void ComputeShader::RecomputeNumberOfGroups (GLuint w, GLuint h, GLuint d, GLuint t_x, GLuint t_y, GLuint t_z)
  {
    num_groups_x = 1;
    if (w > 0)
      num_groups_x = ceil((float)w / (float)t_x);

    num_groups_y = 1;
    if (h > 0)
      num_groups_y = ceil((float)h / (float)t_y);

    num_groups_z = 1;
    if (d > 0)
      num_groups_z = ceil((float)d / (float)t_z);
  }

  void ComputeShader::Dispatch ()
  {
    glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
    gl::ExitOnGLError("ComputeShader: After glDispatchCompute.");
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    gl::ExitOnGLError("ComputeShader: After glMemoryBarrier.");
  }

  void ComputeShader::BindImageTexture (gl::Texture3D* tex, GLuint unit,
                                        GLint level, GLenum access, 
                                        GLenum format, GLboolean layered,
                                        GLint layer)
  {
    glBindTexture(GL_TEXTURE_3D, tex->GetTextureID());
    glBindImageTexture(unit, tex->GetTextureID(), level, layered, layer, access, format);
  }


  void ComputeShader::CompileShader (GLuint shader_id, std::string filename)
  {
    char* shader_source = gl::TextFileRead(filename.c_str());
    const char* const_shader_source = shader_source;

    // Second parameters can be > 1 if const_shader_source is an array.
    glShaderSource(shader_id, 1, &const_shader_source, NULL);
    free(shader_source);

    glCompileShader(shader_id);

    // Check compilation status
    int rvalue;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue) {
      fprintf(stderr, "Error in compiling the compute shader\n");
      GLchar log[10240];
      GLsizei length;
      glGetShaderInfoLog(shader_id, 10239, &length, log);
      fprintf(stderr, "Compiler log:\n%s\n", log);
      exit(40);
    }

    gl::ExitOnGLError("gl::ComputeShader >> Could not compile the shader file!");
  }
}