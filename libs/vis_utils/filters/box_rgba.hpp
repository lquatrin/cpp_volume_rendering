// Codes from "A Fresh Look at Generalized Sampling"
#ifndef IMAGE_FILTERS_BOX_RGBA_H
#define IMAGE_FILTERS_BOX_RGBA_H

#include "kernelbase.hpp"
#include <glm/glm.hpp>

namespace vis
{
  // Simple box kernel
  struct BoxRGBA final : KernelBase<1, glm::vec4> {
    float operator() (float x) const override {
      return x <= -0.5f || x > 0.5f ? 0.0f : 1.0f;
    }
    void accumulate_buffer(float fu, float) override {
      b[0] += fu;
    }
    glm::vec4 sample_buffer(float) const override {
      return b[0];
    }
    void digital_filter(std::vector<glm::vec4>&) const override {
    }
    void digital_filter2D (std::vector<std::vector<glm::vec4>>&) const override {
    }
    std::string name() const override {
      return "Box";
    }
  };
};

#endif