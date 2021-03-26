/**
 * imagefilter.cpp
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#include <volvis_utils/imagefilter.h>

namespace vis
{
  ImageFilter::ImageFilter ()
    : m_cp_reconstruction_filter(nullptr)
    , m_cp_pixel_acc_mean_filter(nullptr)
  {
  }

  ImageFilter::~ImageFilter ()
  {
  }

  void ImageFilter::BuildShaders ()
  {
  }

  void ImageFilter::ApplyReconstructionFilter (gl::Texture2D* screen_output,
                                               gl::Texture2D* screen_input)
  {
    printf("ApplyReconstructionFilter\n");
  }

  void ImageFilter::ApplyPixelAccumulatedMeanFilter (gl::Texture2D* screen_output,
                                                     gl::Texture2D* screen_input,
                                                     int n_rays_per_pixel)
  {
    printf("ApplyReductionFilter\n");
  }
}