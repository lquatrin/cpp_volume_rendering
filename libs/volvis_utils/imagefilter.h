/**
 * imagefilter.h
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef VOL_VIS_UTILS_IMAGE_FILTERS_H
#define VOL_VIS_UTILS_IMAGE_FILTERS_H

#include <iostream>

#include <gl_utils/texture2d.h>
#include <gl_utils/computeshader.h>

namespace vis
{
  class ImageFilter
  {
  public:
    ImageFilter ();
    ~ImageFilter ();

    void BuildShaders ();

    // From reference: 
    // A Fresh Look at Generalized Sampling [2012]
    // Diego Nehab
    // Instituto Nacional de Matemática Pura e Aplicada(IMPA)
    void ApplyReconstructionFilter (gl::Texture2D* screen_output, 
                                    gl::Texture2D* screen_input);

    // The screen_output is a summation of all rays casted from camera
    void ApplyPixelAccumulatedMeanFilter (gl::Texture2D* screen_output, 
                                          gl::Texture2D* screen_input, 
                                          int n_rays_per_pixel);
  protected:
  private:
    gl::ComputeShader* m_cp_reconstruction_filter;
    gl::ComputeShader* m_cp_pixel_acc_mean_filter;

  };
}

#endif