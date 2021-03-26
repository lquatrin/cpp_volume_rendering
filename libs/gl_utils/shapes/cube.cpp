#include "cube.h"

namespace gl
{
  Cube::Cube ()
  {
  }

  Cube::~Cube ()
  {
    Destroy();
  }

  void Cube::Create ()
  {
	  const Vertex VERTICES[8] =
	  {
		  { { -1.0f, -1.0f,  1.0f, 1 }, { 0, 0, 1, 1 } },
		  { { -1.0f,  1.0f,  1.0f, 1 }, { 1, 0, 0, 1 } },
		  { {  1.0f,  1.0f,  1.0f, 1 }, { 0, 1, 0, 1 } },
		  { {  1.0f, -1.0f,  1.0f, 1 }, { 1, 1, 0, 1 } },
		  { { -1.0f, -1.0f, -1.0f, 1 }, { 1, 1, 1, 1 } },
		  { { -1.0f,  1.0f, -1.0f, 1 }, { 1, 0, 0, 1 } },
		  { {  1.0f,  1.0f, -1.0f, 1 }, { 1, 0, 1, 1 } },
		  { {  1.0f, -1.0f, -1.0f, 1 }, { 0, 0, 1, 1 } }
	  };
    
	  const GLuint INDICES[36] =
	  {
		  0,2,1,  0,3,2,
		  4,3,0,  4,7,3,
		  4,1,5,  4,0,1,
		  3,6,2,  3,7,6,
		  1,6,5,  1,2,6,
		  7,5,6,  7,4,5
	  };

	  glGenVertexArrays(1, &BufferIds[0]);
    ExitOnGLError("ERROR: Could not generate the VAO");
	  glBindVertexArray(BufferIds[0]);
	  ExitOnGLError("ERROR: Could not bind the VAO");
    
	  glEnableVertexAttribArray(0);
	  glEnableVertexAttribArray(1);
	  ExitOnGLError("ERROR: Could not enable vertex attributes");
    
	  glGenBuffers(2, &BufferIds[1]);
	  ExitOnGLError("ERROR: Could not generate the buffer objects");
    
	  glBindBuffer(GL_ARRAY_BUFFER, BufferIds[1]);
	  glBufferData(GL_ARRAY_BUFFER, sizeof(VERTICES), VERTICES, GL_STATIC_DRAW);
	  ExitOnGLError("ERROR: Could not bind the VBO to the VAO");
    
	  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)0);
	  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), (GLvoid*)sizeof(VERTICES[0].Position));
	  ExitOnGLError("ERROR: Could not set VAO attributes");
    
	  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferIds[2]);
	  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(INDICES), INDICES, GL_STATIC_DRAW);
	  ExitOnGLError("ERROR: Could not bind the IBO to the VAO");
    
	  glBindVertexArray(0);
  }

  void Cube::Destroy ()
  {
    glDeleteBuffers(2, &BufferIds[1]);
	  glDeleteVertexArrays(1, &BufferIds[0]);
	  ExitOnGLError("ERROR: Could not destroy the buffer objects");	
  }

  void Cube::Draw (void)
  {
    glBindVertexArray(BufferIds[0]);
	  ExitOnGLError("ERROR: Could not bind the VAO for drawing purposes");
    
	  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLvoid*)0);
	  ExitOnGLError("ERROR: Could not draw the cube");
    
	  glBindVertexArray(0);
  }

}