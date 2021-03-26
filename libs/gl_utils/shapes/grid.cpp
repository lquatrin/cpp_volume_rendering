#include "grid.h"

namespace gl
{
  Grid_21::Grid_21 ()
  {
  }

  Grid_21::~Grid_21 ()
  {
    Destroy();
  }

  void Grid_21::Create ()
  {
  }

  void Grid_21::Destroy ()
  {
    glDeleteBuffers(2, &BufferIds[1]);
	  glDeleteVertexArrays(1, &BufferIds[0]);
	  ExitOnGLError("ERROR: Could not destroy the buffer objects");	
  }

  void Grid_21::Draw (void)
  {
  }

}