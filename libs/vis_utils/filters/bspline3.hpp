// Codes from "A Fresh Look at Generalized Sampling"
#ifndef IMAGE_FILTERS_BSPLINE3PIECES_H
#define IMAGE_FILTERS_BSPLINE3PIECES_H

#include "kernelbase.hpp"
#include "utils.hpp"

namespace vis
{
  // Cubic B-spline kernel pieces (multiplied by 6)
  struct Bspline3Pieces {
    static float k0(float u) { return ((u)*u) * u; }
    static float k1(float u) { return ((-3.f * u + 3.f) * u + 3.f) * u + 1.f; }
    static float k2(float u) { return ((3.f * u - 6.f) * u) * u + 4.f; }
    static float k3(float u) { return ((-u + 3.f) * u - 3.f) * u + 1.f; }
  };

  // Generalized Cardinal Cubic B-spline kernel
  struct CardinalBspline3 final : Symmetric4Pieces<Bspline3Pieces, float> {
    void digital_filter(std::vector<float>& f) const override {
      // Pre-factored L U decomposition of digital filter
      const std::array<float, 8> L{ .2f, .26315789f, .26760563f,
      .26792453f, .26794742f, .26794907f, .26794918f, .26794919f };
      linear_solve(L, f);
    }
    void digital_filter2D(std::vector<std::vector<float>>& f) const override {
      // Pre-factored L U decomposition of digital filter
      const std::array<float, 8> L{ .2f, .26315789f, .26760563f,
      .26792453f, .26794742f, .26794907f, .26794918f, .26794919f };

      // for each row
      for (int i = 0; i < f.size(); i++)
        linear_solve(L, f[i]);

      // for each column
      for (int i = 0; i < f[0].size(); i++)
      {
        std::vector<float> col;
        for (int r = 0; r < f.size(); r++)
          col.push_back(f[r][i]);
        linear_solve(L, col);
        for (int r = 0; r < f.size(); r++)
          f[r][i] = col[r];
      }
    }

    std::string name () const override {
      return "Cardinal Cubic B-spline";
    }
    float integral () const override { return 6.f; }
  };
};

#endif