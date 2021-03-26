#include "shader.h"

#include <cstdio>
#include <iostream>
#include <cerrno>

#include <gl_utils/utils.h>

namespace gl
{
  // . UNSIGNED_INT
  static void UniformBind_UNSIGNED_INT (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    glUniform1ui(v->location, (*(GLuint*)v->data));
  }
  
  static void UniformDestroyData_UNSIGNED_INT (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    delete (GLuint*)v->data;
    v->data = nullptr;
  }
  
  // . INT
  static void UniformBind_INT (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    glUniform1i(v->location, (*(GLint*)v->data));
  }
  
  static void UniformDestroyData_INT (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    delete (GLint*)v->data;
    v->data = nullptr;
  }
  
  // . INT_ARRAY
  static void UniformDestroyData_INT_ARRAY (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    delete[] (GLint*)v->data;
    v->data = nullptr;
  }
  
  // . INT_ARRAY
  static void UniformBind_INT_ARRAY (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    glUniform1iv(v->location, v->v_count, (GLint*)v->data);
  }
  
  // . FLOAT
  static void UniformBind_FLOAT (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    glUniform1f(v->location, (*(GLfloat*)v->data));
  }
  
  static void UniformDestroyData_FLOAT (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    delete (GLfloat*)v->data;
    v->data = nullptr;
  }

  // . FLOAT2 FLOAT3 FLOAT4 FLOAT4X4
  static void UniformDestroyData_FLOAT_ARRAY (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    delete[] (GLfloat*)v->data;
    v->data = nullptr;
  }
  
  // . FLOAT2
  static void UniformBind_FLOAT2 (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    GLfloat* udata = (GLfloat*)v->data;
    glUniform2f(v->location, udata[0], udata[1]);
  }
  
  // . FLOAT3
  static void UniformBind_FLOAT3 (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    GLfloat* udata = (GLfloat*)v->data;
    glUniform3f(v->location, udata[0], udata[1], udata[2]);
  }
  
  // . FLOAT4
  static void UniformBind_FLOAT4 (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    GLfloat* udata = (GLfloat*)v->data;
    glUniform4f(v->location, udata[0], udata[1], udata[2], udata[3]);
  }
  
  // . FLOAT4X4
  static void UniformBind_FLOAT4X4 (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    glUniformMatrix4fv(v->location, 1, GL_FALSE, (GLfloat*)v->data);
  }

  // . FLOAT3_ARRAY
  static void UniformBind_FLOAT3_ARRAY (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    glUniform3fv(v->location, v->v_count, (GLfloat*)v->data);
  }
  
  // . FLOAT4_ARRAY
  static void UniformBind_FLOAT4_ARRAY (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    glUniform4fv(v->location, v->v_count, (GLfloat*)v->data);
  }
  
  // . DOUBLE
  static void UniformBind_DOUBLE (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    glUniform1d(v->location, (*(GLdouble*)v->data));
  }
  
  static void UniformDestroyData_DOUBLE (void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    delete (GLdouble*)v->data;
    v->data = nullptr;
  }
  
  // . TEXTURE1D TEXTURE2D TEXTURE3D TEXTURERECTANGLE
  static void UniformDestroyData_TEXTURE_GENERAL(void* unif_data)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
    delete[] (GLuint*)v->data;
    v->data = nullptr;
  }
  
  static void UniformBind_TEXTURE_GENERAL (void* unif_data, GLenum TEXTURE_TYPE)
  {
    UniformVariable* v = (UniformVariable*)unif_data;
  
    GLuint *aux = (GLuint*)v->data;
  
    glActiveTexture(GL_TEXTURE0 + aux[1]);
    glBindTexture(TEXTURE_TYPE, aux[0]);
  
    glUniform1i(v->location, aux[1]);
  
    glActiveTexture(GL_TEXTURE0);
  }
  
  // . TEXTURE1D
  static void UniformBind_TEXTURE1D(void* unif_data)
  {
    UniformBind_TEXTURE_GENERAL (unif_data, GL_TEXTURE_1D);
  }
  
  // . TEXTURE2D
  static void UniformBind_TEXTURE2D (void* unif_data)
  {
    UniformBind_TEXTURE_GENERAL (unif_data, GL_TEXTURE_2D);
  }
  
  // . TEXTURE3D
  static void UniformBind_TEXTURE3D (void* unif_data)
  {
    UniformBind_TEXTURE_GENERAL(unif_data, GL_TEXTURE_3D);
  }
  
  // . TEXTURERECTANGLE
  static void UniformBind_TEXTURERECTANGLE (void* unif_data)
  {
    UniformBind_TEXTURE_GENERAL(unif_data, GL_TEXTURE_RECTANGLE);
  }

  UniformVariable::UniformVariable()
  {
    type     = GLSL_UNIFORM_VARIABLE_TYPES::NONE;
    location = -1;
    data     = nullptr;
    v_count  = -1;

    bind_function        = nullptr;
    destroydata_function = nullptr;
  }

  UniformVariable::~UniformVariable()
  {
    // This is called when setting a new uniform, which may delete the data
    // . we just delete the content in the Shader Class
  }

  void UniformVariable::Bind()
  {
    if (bind_function)
    {
      bind_function(this);
      return;
    }
  }

  void UniformVariable::DestroyData ()
  {
    if (data)
    {
      if (destroydata_function)
      {
        destroydata_function(this);
        data = nullptr;
        return;
      }
      data = nullptr;
    }
  }

  void Shader::Unbind ()
  {
    glUseProgram(0);
  }
  
  Shader::Shader ()
  {
    shader_program = -1;
    uniform_variables.clear();
  }
  
  Shader::~Shader ()
  {
    ClearUniforms();
    uniform_variables.clear();
  
    glDeleteProgram(shader_program);
    shader_program = -1;
  }
  
  void Shader::Bind ()
  {
    glUseProgram(shader_program);
    gl::ExitOnGLError("Error at Shader::Bind()");
  }
  
  GLuint Shader::GetProgramID ()
  {
    return shader_program;
  }
  
  GLint Shader::GetUniformLoc (char* name)
  {
    int uniform_location = glGetUniformLocation(shader_program, name);
    return uniform_location;
  }
  
  GLint Shader::GetAttribLoc (char* name)
  {
    int attrib_location = glGetAttribLocation(shader_program, name);
    return attrib_location;
  }
  
  void Shader::BindUniforms ()
  {
    for (std::map<std::string, UniformVariable>::iterator it = uniform_variables.begin(); it != uniform_variables.end(); ++it)
      uniform_variables[it->first].Bind();
    gl::ExitOnGLError("Error at BindUniforms()");
  }
  
  void Shader::ClearUniforms ()
  {
    for (std::map<std::string, UniformVariable>::iterator it = uniform_variables.begin(); it != uniform_variables.end(); ++it)
    {
      it->second.DestroyData();
    }
    gl::ExitOnGLError("Error at ClearUniforms()");
    uniform_variables.clear();
  }
  
  void Shader::BindUniform (std::string var_name)
  {
    if (uniform_variables.find(var_name) != uniform_variables.end())
    {
      uniform_variables[var_name].Bind();
      gl::ExitOnGLError("Error at Shader::BindUniform()");
    }
    else
    {
      std::cout << "Shader: Uniform " << var_name << " not found!" << std::endl;
    }
  }
  
  void Shader::ClearUniform (std::string var_name)
  {
    for (std::map<std::string, UniformVariable>::iterator it = uniform_variables.begin(); it != uniform_variables.end(); ++it)
    {
      if (it->first.compare(var_name) == 0)
      {
        it->second.DestroyData();
        uniform_variables.erase(it);
        break;
      }
    }

    for (std::map<std::string, UniformVariable>::iterator it = uniform_variables.begin(); it != uniform_variables.end(); ++it)
    {
      std::cout << it->first << std::endl;
    }
    gl::ExitOnGLError("gl::Shader: Error at ClearUniform()");
  }

  // unsigned int = GLuint
  void Shader::SetUniform (std::string name, unsigned int value)
  {
    bool unif_found = (uniform_variables.find(name) != uniform_variables.end());
  
    // TODO: if "force_override" is false
  
    // create uniform
    GLuint* input_uniform = new GLuint();
    *input_uniform = (GLuint)value;
  
    // if we found the uniform, delete the current data
    if (unif_found)
    {
      uniform_variables[name].DestroyData();
      uniform_variables[name].data = input_uniform;
    }
    // else, create the new uniform
    else
    {
      UniformVariable ul;
      ul.type = GLSL_UNIFORM_VARIABLE_TYPES::UNSIGNED_INT;
      ul.location = glGetUniformLocation(shader_program, name.c_str());
      ul.data = input_uniform;
      ul.v_count = -1;
  
      ul.bind_function = UniformBind_UNSIGNED_INT;
      ul.destroydata_function = UniformDestroyData_UNSIGNED_INT;
  
      // insert into the uniform map
      uniform_variables.insert(std::pair<std::string, UniformVariable>(name, ul));
    }
    gl::ExitOnGLError("Error on SetUniform [UNSIGNED_INT]");
  }
  
  void Shader::SetUniform (std::string name, int value)
  {
    bool unif_found = (uniform_variables.find(name) != uniform_variables.end());
  
    // TODO: if "force_override" is false
  
    // create uniform
    GLint* input_uniform = new GLint();
    *input_uniform = (GLint)value;
  
    // if we found the uniform, delete the current data
    if (unif_found)
    {
      uniform_variables[name].DestroyData();
      uniform_variables[name].data = input_uniform;
    }
    // else, create the new uniform
    else
    {
      UniformVariable ul;
      ul.type = GLSL_UNIFORM_VARIABLE_TYPES::INT;
      ul.location = glGetUniformLocation(shader_program, name.c_str());
      ul.data = input_uniform;
      ul.v_count = -1;
  
      ul.bind_function = UniformBind_INT;
      ul.destroydata_function = UniformDestroyData_INT;
  
      // insert into the uniform map
      uniform_variables.insert(std::pair<std::string, UniformVariable>(name, ul));
    }
    gl::ExitOnGLError("Error on SetUniform [INT]");
  }
  
  void Shader::SetUniformArray (std::string name, std::vector<int> value)
  {
    bool unif_found = (uniform_variables.find(name) != uniform_variables.end());
  
    // TODO: if "force_override" is false
  
    // create uniform
    GLint* input_array = new GLint[value.size()];
    for (int i = 0; i < value.size(); i++)
      input_array[i] = (GLint)value[i];
  
    // if we found the uniform, delete the current data
    if (unif_found)
    {
      uniform_variables[name].DestroyData();
      uniform_variables[name].data = input_array;
    }
    // else, create the new uniform
    else
    {
      UniformVariable ul;
      ul.type = GLSL_UNIFORM_VARIABLE_TYPES::INT_ARRAY;
      ul.location = glGetUniformLocation(shader_program, name.c_str());
      ul.data = input_array;
      ul.v_count = value.size();
  
      ul.bind_function = UniformBind_INT_ARRAY;
      ul.destroydata_function = UniformDestroyData_INT_ARRAY;
  
      // insert into the uniform map
      uniform_variables.insert(std::pair<std::string, UniformVariable>(name, ul));
    }
    gl::ExitOnGLError("Error on SetUniform [ARRAY INT]");
  }
  
  void Shader::SetUniform (std::string name, float value)
  {
    bool unif_found = (uniform_variables.find(name) != uniform_variables.end());
  
    // TODO: if "force_override" is false
  
    // create uniform
    GLfloat* input_uniform = new GLfloat();
    *input_uniform = (GLfloat)value;
  
    // if we found the uniform, delete the current data
    if (unif_found)
    {
      uniform_variables[name].DestroyData();
      uniform_variables[name].data = input_uniform;
    }
    // else, create the new uniform
    else
    {
      UniformVariable ul;
      ul.type = GLSL_UNIFORM_VARIABLE_TYPES::FLOAT;
      ul.location = glGetUniformLocation(shader_program, name.c_str());
      ul.data = input_uniform;
      ul.v_count = -1;
  
      ul.bind_function = UniformBind_FLOAT;
      ul.destroydata_function = UniformDestroyData_FLOAT;
  
      // insert into the uniform map
      uniform_variables.insert(std::pair<std::string, UniformVariable>(name, ul));
    }
    gl::ExitOnGLError("Error on SetUniform [FLOAT]");
  }
  
  void Shader::SetUniform (std::string name, glm::vec2 value)
  {
    bool unif_found = (uniform_variables.find(name) != uniform_variables.end());
  
    // TODO: if "force_override" is false
  
    // create uniform
    GLfloat* input_uniform = new GLfloat[2];
    input_uniform[0] = (GLfloat)value.x;
    input_uniform[1] = (GLfloat)value.y;
  
    // if we found the uniform, delete the current data
    if (unif_found)
    {
      uniform_variables[name].DestroyData();
      uniform_variables[name].data = input_uniform;
    }
    // else, create the new uniform
    else
    {
      UniformVariable ul;
      ul.type = GLSL_UNIFORM_VARIABLE_TYPES::FLOAT2;
      ul.location = glGetUniformLocation(shader_program, name.c_str());
      ul.data = input_uniform;
      ul.v_count = -1;
  
      ul.bind_function = UniformBind_FLOAT2;
      ul.destroydata_function = UniformDestroyData_FLOAT_ARRAY;
  
      // insert into the uniform map
      uniform_variables.insert(std::pair<std::string, UniformVariable>(name, ul));
    }
    gl::ExitOnGLError("Error on SetUniform [GLM FLOAT2]");
  }
  //
  void Shader::SetUniform (std::string name, glm::vec3 value)
  {
    bool unif_found = (uniform_variables.find(name) != uniform_variables.end());
  
    // TODO: if "force_override" is false
  
    // create uniform
    GLfloat* input_uniform = new GLfloat[3];
    input_uniform[0] = (GLfloat)value.x;
    input_uniform[1] = (GLfloat)value.y;
    input_uniform[2] = (GLfloat)value.z;
  
    if (unif_found)
    {
      uniform_variables[name].DestroyData();
      uniform_variables[name].data = input_uniform;
    }
    // else, create the new uniform
    else
    {
      UniformVariable ul;
      ul.type = GLSL_UNIFORM_VARIABLE_TYPES::FLOAT3;
      ul.location = glGetUniformLocation(shader_program, name.c_str());
      ul.data = input_uniform;
      ul.v_count = -1;
   
      ul.bind_function = UniformBind_FLOAT3;
      ul.destroydata_function = UniformDestroyData_FLOAT_ARRAY;
  
      // insert into the uniform map
      uniform_variables.insert(std::pair<std::string, UniformVariable>(name, ul));
    }
    gl::ExitOnGLError("Error on SetUniform [GLM FLOAT3]");
  }
  
  void Shader::SetUniform (std::string name, glm::vec4 value)
  {
    bool unif_found = (uniform_variables.find(name) != uniform_variables.end());
  
    // TODO: if "force_override" is false
  
    // create uniform
    GLfloat* input_uniform = new GLfloat[4];
    input_uniform[0] = (GLfloat)value.x;
    input_uniform[1] = (GLfloat)value.y;
    input_uniform[2] = (GLfloat)value.z;
    input_uniform[3] = (GLfloat)value.w;
  
    if (unif_found)
    {
      uniform_variables[name].DestroyData();
      uniform_variables[name].data = input_uniform;
    }
    // else, create the new uniform
    else
    {
      UniformVariable ul;
      ul.type = GLSL_UNIFORM_VARIABLE_TYPES::FLOAT4;
      ul.location = glGetUniformLocation(shader_program, name.c_str());
      ul.data = input_uniform;
      ul.v_count = -1;
  
      ul.bind_function = UniformBind_FLOAT4;
      ul.destroydata_function = UniformDestroyData_FLOAT_ARRAY;
  
      // insert into the uniform map
      uniform_variables.insert(std::pair<std::string, UniformVariable>(name, ul));
    }
    gl::ExitOnGLError("Error on SetUniform [GLM FLOAT4]");
  }
  
  void Shader::SetUniform (std::string name, glm::mat4 value)
  {
    bool unif_found = (uniform_variables.find(name) != uniform_variables.end());
  
    // TODO: if "force_override" is false
  
    // input uniform in column major order
    GLfloat* input_uniform = new GLfloat[16];
    input_uniform[0]  = (GLfloat)value[0][0];
    input_uniform[1]  = (GLfloat)value[0][1];
    input_uniform[2]  = (GLfloat)value[0][2];
    input_uniform[3]  = (GLfloat)value[0][3];
    input_uniform[4]  = (GLfloat)value[1][0];
    input_uniform[5]  = (GLfloat)value[1][1];
    input_uniform[6]  = (GLfloat)value[1][2];
    input_uniform[7]  = (GLfloat)value[1][3];
    input_uniform[8]  = (GLfloat)value[2][0];
    input_uniform[9]  = (GLfloat)value[2][1];
    input_uniform[10] = (GLfloat)value[2][2];
    input_uniform[11] = (GLfloat)value[2][3];
    input_uniform[12] = (GLfloat)value[3][0];
    input_uniform[13] = (GLfloat)value[3][1];
    input_uniform[14] = (GLfloat)value[3][2];
    input_uniform[15] = (GLfloat)value[3][3];
  
    if (unif_found)
    {
      uniform_variables[name].DestroyData();
      uniform_variables[name].data = input_uniform;
    }
    // else, create the new uniform
    else
    {
      UniformVariable ul;
  
      ul.type = GLSL_UNIFORM_VARIABLE_TYPES::FLOAT4X4;
      ul.location = glGetUniformLocation(shader_program, name.c_str());
      ul.data = input_uniform;
      ul.v_count = -1;
  
      ul.bind_function = UniformBind_FLOAT4X4;
      ul.destroydata_function = UniformDestroyData_FLOAT_ARRAY;
  
      // insert into the uniform map
      uniform_variables.insert(std::pair<std::string, UniformVariable>(name, ul));
    }
    gl::ExitOnGLError("Error on SetUniform [GLM FLOAT4X4]");
  }
  
  void Shader::SetUniformMatrix4f (std::string name, Matrix4f value)
  {
    bool unif_found = (uniform_variables.find(name) != uniform_variables.end());
  
    // TODO: if "force_override" is false
  
    GLfloat* input_uniform = new GLfloat[16];
    input_uniform[0]  = value.m[0];  input_uniform[1]  = value.m[1];
    input_uniform[2]  = value.m[2];  input_uniform[3]  = value.m[3];
    input_uniform[4]  = value.m[4];  input_uniform[5]  = value.m[5];
    input_uniform[6]  = value.m[6];  input_uniform[7]  = value.m[7];
    input_uniform[8]  = value.m[8];  input_uniform[9]  = value.m[9];
    input_uniform[10] = value.m[10]; input_uniform[11] = value.m[11];
    input_uniform[12] = value.m[12]; input_uniform[13] = value.m[13];
    input_uniform[14] = value.m[14]; input_uniform[15] = value.m[15];
  
    // if we found the uniform, delete the current data
    if (unif_found)
    {
      uniform_variables[name].DestroyData();
      uniform_variables[name].data = input_uniform;
    }
    // else, create the new uniform
    else
    {
      UniformVariable ul;
  
      ul.type = GLSL_UNIFORM_VARIABLE_TYPES::FLOAT4X4;
      ul.location = glGetUniformLocation(shader_program, name.c_str());
      ul.data = input_uniform;
      ul.v_count = -1;
  
      ul.bind_function = UniformBind_FLOAT4X4;
      ul.destroydata_function = UniformDestroyData_FLOAT_ARRAY;
  
      // insert into the uniform map
      uniform_variables.insert(std::pair<std::string, UniformVariable>(name, ul));
    }
    gl::ExitOnGLError("Error on SetUniform [MATRIX4F]");
  }
  
  void Shader::SetUniformArray (std::string name, std::vector<glm::vec3> value)
  {
    bool unif_found = (uniform_variables.find(name) != uniform_variables.end());
  
    // TODO: if "force_override" is false
    
    GLfloat* input_array = new GLfloat[value.size() * 3];
    for (int i = 0; i < value.size(); i++)
    {
      input_array[i * 3 + 0] = value[i].x;
      input_array[i * 3 + 1] = value[i].y;
      input_array[i * 3 + 2] = value[i].z;
    }
  
    // if we found the uniform, delete the current data
    if (unif_found)
    {
      uniform_variables[name].DestroyData();
      uniform_variables[name].data = input_array;
    }
    // else, create the new uniform
    else
    {
      UniformVariable ul;
  
      ul.type = GLSL_UNIFORM_VARIABLE_TYPES::FLOAT3_ARRAY;
      ul.location = glGetUniformLocation(shader_program, name.c_str());
      ul.data = input_array;
      ul.v_count = value.size();
  
      ul.bind_function = UniformBind_FLOAT3_ARRAY;
      ul.destroydata_function = UniformDestroyData_FLOAT_ARRAY;
  
      // insert into the uniform map
      uniform_variables.insert(std::pair<std::string, UniformVariable>(name, ul));
    }
    gl::ExitOnGLError("Error on SetUniform [ARRAY FLOAT3]");
  }
  
  void Shader::SetUniformArray (std::string name, std::vector<glm::vec4> value)
  {
    bool unif_found = (uniform_variables.find(name) != uniform_variables.end());
  
    // TODO: if "force_override" is false
  
    GLfloat* input_array = new GLfloat[value.size() * 4];
    for (int i = 0; i < value.size(); i++)
    {
      input_array[i * 4 + 0] = value[i].x;
      input_array[i * 4 + 1] = value[i].y;
      input_array[i * 4 + 2] = value[i].z;
      input_array[i * 4 + 3] = value[i].w;
    }
  
    // if we found the uniform, delete the current data
    if (unif_found)
    {
      uniform_variables[name].DestroyData();
      uniform_variables[name].data = input_array;
    }
    // else, create the new uniform
    else
    {
      UniformVariable ul;
  
      ul.type = GLSL_UNIFORM_VARIABLE_TYPES::FLOAT4_ARRAY;
      ul.location = glGetUniformLocation(shader_program, name.c_str());
      ul.data = input_array;
      ul.v_count = value.size();
  
      ul.bind_function = UniformBind_FLOAT4_ARRAY;
      ul.destroydata_function = UniformDestroyData_FLOAT_ARRAY;
  
      // insert into the uniform map
      uniform_variables.insert(std::pair<std::string, UniformVariable>(name, ul));
    }
    gl::ExitOnGLError("Error on SetUniform [ARRAY FLOAT4]");
  }
  
  void Shader::SetUniform (std::string name, double value)
  {
    bool unif_found = (uniform_variables.find(name) != uniform_variables.end());
  
    // TODO: if "force_override" is false
  
    // create uniform
    GLdouble* input_uniform = new GLdouble();
    *input_uniform = (GLdouble)value;
  
    // if we found the uniform, delete the current data
    if (unif_found)
    {
      uniform_variables[name].DestroyData();
      uniform_variables[name].data = input_uniform;
    }
    // else, create the new uniform
    else
    {
      UniformVariable ul;
      ul.type = GLSL_UNIFORM_VARIABLE_TYPES::DOUBLE;
      ul.location = glGetUniformLocation(shader_program, name.c_str());
      ul.data = input_uniform;
      ul.v_count = -1;
  
      ul.bind_function = UniformBind_DOUBLE;
      ul.destroydata_function = UniformDestroyData_DOUBLE;
  
      // insert into the uniform map
      uniform_variables.insert(std::pair<std::string, UniformVariable>(name, ul));
    }
    gl::ExitOnGLError("Error on SetUniform [DOUBLE]");
  }
  
  void Shader::SetUniformTexture1D (std::string name, GLuint texture_unit_id, GLuint active_texture_id)
  {
    SetUniformTexture(name, texture_unit_id, active_texture_id, GLSL_UNIFORM_VARIABLE_TYPES::TEXTURE1D,
      UniformBind_TEXTURE1D, UniformDestroyData_TEXTURE_GENERAL);
  }
  
  void Shader::SetUniformTexture2D (std::string name, GLuint texture_unit_id, GLuint active_texture_id)
  {
    SetUniformTexture(name, texture_unit_id, active_texture_id, GLSL_UNIFORM_VARIABLE_TYPES::TEXTURE2D,
      UniformBind_TEXTURE2D, UniformDestroyData_TEXTURE_GENERAL);
  }
  
  void Shader::SetUniformTexture3D (std::string name, GLuint texture_unit_id, GLuint active_texture_id)
  {
    SetUniformTexture(name, texture_unit_id, active_texture_id, GLSL_UNIFORM_VARIABLE_TYPES::TEXTURE3D,
      UniformBind_TEXTURE3D, UniformDestroyData_TEXTURE_GENERAL);
  }
  
  void Shader::SetUniformTextureRectangle (std::string name, GLuint texture_unit_id, GLuint active_texture_id)
  {
    SetUniformTexture(name, texture_unit_id, active_texture_id, GLSL_UNIFORM_VARIABLE_TYPES::TEXTURERECTANGLE,
      UniformBind_TEXTURERECTANGLE, UniformDestroyData_TEXTURE_GENERAL);
  }
  
  void Shader::SetUniformTexture (std::string name,
                                  GLuint texture_unit_id,
                                  GLuint active_texture_id,
                                  GLSL_UNIFORM_VARIABLE_TYPES type,
                                  UniformFunction bind_func,
                                  UniformFunction destroydata_func)
  {
    bool unif_found = (uniform_variables.find(name) != uniform_variables.end());
  
    // TODO: if "force_override" is false
  
    // create uniform
    GLuint* input_uniform = new GLuint[2];
    input_uniform[0] = texture_unit_id;
    input_uniform[1] = active_texture_id;
    
    if (unif_found)
    {
      uniform_variables[name].DestroyData();
      uniform_variables[name].data = input_uniform;
    }
    // else, create the new uniform
    else
    {
      UniformVariable ul;
      ul.type = type;
      ul.location = glGetUniformLocation(shader_program, name.c_str());
      ul.data = input_uniform;
      ul.v_count = -1;
  
      ul.bind_function = bind_func;
      ul.destroydata_function = destroydata_func;
  
      // insert into the uniform map
      uniform_variables.insert(std::pair<std::string, UniformVariable>(name, ul));
    }
    gl::ExitOnGLError("Error on SetUniform [TEXTURE]");
  }
}