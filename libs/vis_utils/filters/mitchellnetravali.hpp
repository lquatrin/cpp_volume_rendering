// Codes from "A Fresh Look at Generalized Sampling"
#ifndef IMAGE_FILTERS_MITCHELLNETRAVALI_H
#define IMAGE_FILTERS_MITCHELLNETRAVALI_H

#include "kernelbase.hpp"
#include "utils.hpp"

namespace vis
{
  // Traditional Mitchell-Netravali kernel
  struct MitchellNetravaliPieces {
    static float k0(float u) {
      return (((7 / 18.0f) * u - 1 / 3.0f) * u) * u;
    }
    static float k1(float u) {
      return (((-7 / 6.0f) * u + 1.5f) * u + 0.5f) * u + 1 / 18.0f;
    }
    static float k2(float u) {
      return (((7 / 6.0f) * u - 2.0f) * u) * u + 8 / 9.0f;
    }
    static float k3(float u) {
      return (((-7 / 18.0f) * u + 5 / 6.0f) * u - 0.5f) * u + 1 / 18.0f;
    }
  };

  struct MitchellNetravali final : Symmetric4Pieces<MitchellNetravaliPieces, float> {
    void digital_filter(std::vector<float>&) const override {
    }
    void digital_filter2D(std::vector<std::vector<float>>&) const override {
    }
    std::string name() const override {
      return "Mitchell-Netravali";
    }
  };

};

#endif