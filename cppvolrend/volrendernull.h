/**
 * Base class for volume rendering of structured datasets.
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef NULL_VOLUME_RENDERER
#define NULL_VOLUME_RENDERER

#include "defines.h"
#include "volrenderbase.h"

class NullRenderer : public BaseVolumeRenderer
{
public:
  NullRenderer ();
  ~NullRenderer ();
  
  virtual const char* GetName ();
  virtual const char* GetAbbreviationName ();

  virtual void Clean ();
  virtual bool Init (int shader_width, int shader_height);

  virtual bool Update (vis::Camera* camera);
  virtual void Redraw ();
  
  virtual vis::GRID_VOLUME_DATA_TYPE GetDataTypeSupport ();
  virtual void SetImGuiComponents ();

protected:
  glm::mat4 view_matrix, projection_matrix;

private:
};

#endif