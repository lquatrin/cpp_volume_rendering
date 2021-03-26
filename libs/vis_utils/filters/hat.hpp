// Codes from "A Fresh Look at Generalized Sampling"
#ifndef IMAGE_FILTERS_HAT_H
#define IMAGE_FILTERS_HAT_H

#include "kernelbase.hpp"

namespace vis
{
  // Simple hat kernel
  struct Hat final : KernelBase<2, float> {
    float operator() (float x) const override {
      x = abs(x);
      return x > 1.f ? 0.0f : 1.0f - x;
    }
    void accumulate_buffer (float fu, float u) override {
      b[0] += fu * (1.0f - u);
      b[1] += fu * u;
    }
    float sample_buffer (float u) const override {
      return b[0] * (1.0f - u) + b[1] * u;
    }
    void digital_filter (std::vector<float>&) const override {
    }
    void digital_filter2D(std::vector<std::vector<float>>&) const override {
    }
    std::string name () const override {
      return "Hat";
    }
  };
};

#endif