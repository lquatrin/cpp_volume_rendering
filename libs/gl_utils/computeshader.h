/**
 * OpenGL General Purpose Compute Shader Class
 * - Reload support
 *
 * About glBindImageTexture and glBindTexture:
 * . https://stackoverflow.com/questions/37136813/what-is-the-difference-between-glbindimagetexture-and-glbindtexture
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef GL_UTILS_COMPUTE_SHADER_H
#define GL_UTILS_COMPUTE_SHADER_H

#include <GL/glew.h>

#include "shader.h"

#include <iostream>
#include <string>

#include "texture3d.h"

namespace gl
{
  class ComputeShader : public Shader
  {
  public:
    ComputeShader ();
    ~ComputeShader ();

    virtual bool LoadAndLink ();
    virtual bool Reload ();

    void SetShaderFile (std::string filename);
    void AddShaderFile (std::string filepath);

    void RecomputeNumberOfGroups (GLuint w, GLuint h, GLuint d, GLuint t_x = 8, GLuint t_y = 8, GLuint t_z = 8);
    
    void Dispatch ();

    // Bind a gl::Texture3D using 'glBindImageTexture'. layered must be true.
    void BindImageTexture (gl::Texture3D* tex, GLuint unit, GLint level, GLenum access,
                           GLenum format, GLboolean layered = GL_TRUE, GLint layer = 0);

  protected:

  private:
    std::vector<std::string> vec_compute_shader_names;
    std::vector<GLuint> vec_compute_shader_ids;

    void CompileShader (GLuint shader_id, std::string filename);

    GLuint num_groups_x;
    GLuint num_groups_y;
    GLuint num_groups_z;

  };
}

#endif