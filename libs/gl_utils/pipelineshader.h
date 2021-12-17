/**
 * OpenGL Pipeline Shader Class
 * - Reload support
 * - Attachment of a list of shaders
 * - Vertex, Fragment and Geometry shader
 *
 * References:
 * . Implementation based on Nvidia dual depth peeling sample from Louis Bavoli ('GLSLProgramObject')
 *   -> http://developer.download.nvidia.com/SDK/10/opengl/screenshots/samples/dual_depth_peeling.html
 *
 * . Shader implementation by Cesar Tadeu Pozzer
 *   -> http://www-usr.inf.ufsm.br/~pozzer/
 *
 * . Thanks to Eduardo Ceretta Dalla Favera for advices about shader reload.
 * 
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef GL_UTILS_PIPELINE_SHADER_H
#define GL_UTILS_PIPELINE_SHADER_H

#include <gl_utils/utils.h>
#include <math_utils/matrix.h>
#include <math_utils/matrix4.h>

#include <glm/glm.hpp>

#include <map>
#include <vector>
#include <iostream>

#include <gl_utils/shader.h>

namespace gl
{ 
  class PipelineShader : public Shader
  {
  public:
    enum TYPE
    {
      VERTEX = 0,
      FRAGMENT = 1,
      GEOMETRY = 2,
    };

    static void CompileShader(GLuint shader, std::string filename);

    PipelineShader ();
    PipelineShader (std::string vert, std::string frag);
    PipelineShader (char *vert, char *frag, char* geom = NULL);
    ~PipelineShader ();

    virtual bool LoadAndLink ();
    virtual bool Reload ();
    
    bool AddShaderFile(TYPE type, std::string filename);

    void Clear();

    enum GS_INPUT
    {
      IN_POINTS = GL_POINTS,
      IN_LINES = GL_LINES,
      IN_LINES_ADJACENCY = GL_LINES_ADJACENCY,
      IN_TRIANGLES = GL_TRIANGLES,
      IN_TRIANGLES_ADJACENCY = GL_TRIANGLES_ADJACENCY
    };

    enum GS_OUTPUT
    {
      OUT_POINTS = GL_POINTS,
      OUT_LINE_STRIP = GL_LINE_STRIP,
      OUT_TRIANGLE_STRIP = GL_TRIANGLE_STRIP
    };

    void SetGeometryShaderPrimitives(GLenum in, GLenum out);
    void SetGeometryShaderPrimitives(PipelineShader::GS_INPUT in, PipelineShader::GS_OUTPUT out);

  private:
    std::vector<std::string> vec_vertex_shaders_names;
    std::vector<std::string> vec_fragment_shaders_names;
    std::vector<std::string> vec_geometry_shaders_names;

    std::vector<GLuint> vec_vertex_shaders_ids;
    std::vector<GLuint> vec_fragment_shaders_ids;
    std::vector<GLuint> vec_geometry_shaders_ids;

    GLenum m_geometryPrimitiveIn;
    GLenum m_geometryPrimitiveOut;

    bool m_hasGeometryShader;
  };
}

#endif