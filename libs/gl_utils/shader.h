/**
 * OpenGL Base Shader Class
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef GL_UTILS_SHADER_H
#define GL_UTILS_SHADER_H

#include <GL/glew.h>

#include <map>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include <math_utils/matrix4.h>

namespace gl
{   
  enum GLSL_UNIFORM_VARIABLE_TYPES
  {
    // INT
    UNSIGNED_INT,
    INT,
    INT_ARRAY,
    
    // FLOAT
    FLOAT,
    FLOAT2,
    FLOAT3,
    FLOAT4,
    FLOAT4X4,
    FLOAT3_ARRAY,
    FLOAT4_ARRAY,
    
    // DOUBLE
    DOUBLE,
    
    // TEXTURE
    TEXTURE1D,
    TEXTURE2D,
    TEXTURE3D,
    TEXTURERECTANGLE,
    
    NONE,
  };
  
  typedef void(*UniformFunction) (void* unif_data);

  class UniformVariable
  {
  public:
    UniformVariable ();
    ~UniformVariable ();

    void Bind ();
    void DestroyData ();

    GLSL_UNIFORM_VARIABLE_TYPES type;
    GLuint location;
    void* data;
    GLuint v_count;

    UniformFunction bind_function;
    UniformFunction destroydata_function;
  };

  class Shader
  {
  public:
    static void Unbind ();
  
    Shader ();
    virtual ~Shader ();
  
    virtual bool LoadAndLink () = 0;
    virtual bool Reload () = 0;
  
    void Bind ();
    GLuint GetProgramID ();
  
    ////////////////////////////////////////////////
    // Uniforms functions
    GLint GetUniformLoc (char* name);
    GLint GetAttribLoc (char* name); 
  
    void BindUniforms ();
    void ClearUniforms ();
    void BindUniform (std::string var_name);
    void ClearUniform (std::string var_name);

    // INT
    void SetUniform (std::string name, unsigned int value);
    void SetUniform (std::string name, int value);
    void SetUniformArray (std::string name, std::vector<int> value);
   
    // FLOAT  
    void SetUniform (std::string name, float value);
    void SetUniform (std::string name, glm::vec2 value);
    void SetUniform (std::string name, glm::vec3 value);
    void SetUniform (std::string name, glm::vec4 value);
    void SetUniform (std::string name, glm::mat4 value);
  
    void SetUniformMatrix4f (std::string name, Matrix4f value);
    
    void SetUniformArray (std::string name, std::vector<glm::vec3> value);
    void SetUniformArray (std::string name, std::vector<glm::vec4> value);
    
    // DOUBLE
    void SetUniform (std::string name, double value);
    
    // TEXTURE
    void SetUniformTexture1D (std::string name, GLuint texture_unit_id, GLuint active_texture_id);
    void SetUniformTexture2D (std::string name, GLuint texture_unit_id, GLuint active_texture_id);
    void SetUniformTexture3D (std::string name, GLuint texture_unit_id, GLuint active_texture_id);
    void SetUniformTextureRectangle (std::string name, GLuint texture_unit_id, GLuint active_texture_id);
    
    protected:
      GLuint shader_program;
      std::map<std::string, UniformVariable> uniform_variables;
  
      void SetUniformTexture (std::string name,
                              GLuint texture_unit_id,
                              GLuint active_texture_id, 
                              GLSL_UNIFORM_VARIABLE_TYPES type,
                              UniformFunction bind_func,
                              UniformFunction destroydata_func);
    
    private:
  };
}

#endif