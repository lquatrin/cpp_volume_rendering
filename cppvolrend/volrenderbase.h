/**
 * Base class for volume rendering of structured datasets.
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef VOLUME_RENDERER
#define VOLUME_RENDERER

#include <volvis_utils/structuredgridvolume.h>
#include <volvis_utils/transferfunction.h>

#include <gl_utils/texture2d.h>
#include <gl_utils/pipelineshader.h>
#include <gl_utils/computeshader.h>

#include <vis_utils/camera.h>
#include <vis_utils/renderoutputframe.h>

#include <volvis_utils/datamanager.h>
#include <volvis_utils/renderingparameters.h>

class BaseVolumeRenderer
{
public:
  enum MULTISCALING {
    SINGLE_RAY_PER_PIXEL = 0,
    MULTIPLE_RAYS_PER_PIXEL = 1,
    DOWN_SCALING_RENDER = 2,
    UP_SCALING_RENDER = 3,
  };
  BaseVolumeRenderer ();
  ~BaseVolumeRenderer ();

  void SetExternalResources (vis::DataManager* data_mgr, vis::RenderingParameters* rdr_prm);

  //////////////////////////////////////////
  // Virtual functions
  virtual const char* GetName () = 0;
  virtual const char* GetAbbreviationName () = 0;

  virtual void Clean ();
  virtual void ReloadShaders ();
  
  virtual bool Init (int shader_width, int shader_height) = 0;
  virtual bool Update (vis::Camera* camera) = 0;
  virtual void Redraw ();
  virtual void MultiSampleRedraw ();
  virtual void DownScalingRedraw ();
  virtual void UpScalingRedraw ();

  virtual void Reshape (int w, int h);
  
  virtual void SetImGuiComponents ();

  virtual vis::GRID_VOLUME_DATA_TYPE GetDataTypeSupport () = 0;
  
  void PrepareRender (vis::Camera* camera);
    
  virtual void SetOutdated ();
  bool IsOutdated ();
  
  bool IsBuilt ();
  
  bool IsPixelMultiScalingSupported ();
  int GetCurrentMultiScalingMode ();
  void SetCurrentMultiScalingMode (int f);

  virtual int GetScreenTextureID () {
    return m_rdr_frame_to_screen.GetScreenOutputTexture()->GetTextureID();
  }

protected:
  void SetBuilt (bool b_built);
  bool AddImGuiMultiSampleOptions ();

  //////////////////////////////////////////
  // State Variables
  bool vr_built;
  bool vr_outdated;
  bool vr_pixel_multiscaling_support;
  int vr_pixel_multiscaling_mode;

  //////////////////////////////////////////
  // External Resources
  vis::DataManager* m_ext_data_manager;
  vis::RenderingParameters* m_ext_rendering_parameters;
  
  //////////////////////////////////////////
  // Render Screen Texture
  vis::RenderFrameToScreen m_rdr_frame_to_screen;

private:
};

#endif