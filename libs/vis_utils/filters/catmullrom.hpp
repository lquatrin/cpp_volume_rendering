// Codes from "A Fresh Look at Generalized Sampling"
#ifndef IMAGE_FILTERS_CATMULLROM_H
#define IMAGE_FILTERS_CATMULLROM_H

#include "kernelbase.hpp"
#include "utils.hpp"

namespace vis
{

  // Traditional Catmull-Rom kernel
  struct CatmullRomPieces {
    static float k0(float u) { return ((.5f * u - .5f) * u) * u; }
    static float k1(float u) { return ((-1.5f * u + 2.f) * u + .5f) * u; }
    static float k2(float u) { return ((1.5f * u - 2.5f) * u) * u + 1.f; }
    static float k3(float u) { return ((-.5 * u + 1.f) * u - .5f) * u; }
  };

  struct CatmullRom final : Symmetric4Pieces<CatmullRomPieces, float> {
    void digital_filter (std::vector<float>&) const override { }
    void digital_filter2D(std::vector<std::vector<float>>&) const override {
    }

    std::string name () const override { return "Catmull-Rom"; }
  };
};

#endif