// Codes from "A Fresh Look at Generalized Sampling"
#ifndef IMAGE_FILTERS_OMOMS3PIECES_RGBA_H
#define IMAGE_FILTERS_OMOMS3PIECES_RGBA_H

#include "kernelbase.hpp"
#include "utils.hpp"
#include <glm/glm.hpp>

namespace vis
{
  // Cubic OMOMS kernel pieces (multiplied by 5.25)
  struct OMOMS3Pieces {
    static float k0(float u) {
      return ((.875f * u) * u + .125f) * u;
    }
    static float k1(float u) {
      return ((-2.625f * u + 2.625f) * u + 2.25f) * u + 1.f;
    }
    static float k2(float u) {
      return ((2.625f * u - 5.25f) * u + .375f) * u + 3.25f;
    }
    static float k3(float u) {
      return ((-.875f * u + 2.625f) * u - 2.75f) * u + 1.f;
    }
  };

  // Generalized Cardinal Cubic O-MOMS3 kernel
  struct CardinalOMOMS3RGBA final : Symmetric4Pieces<OMOMS3Pieces, glm::vec4> {
    void digital_filter(std::vector<glm::vec4>& f) const override {
      // Pre-factored L U decomposition of digital filter
      const std::array<float, 9> L{ .23529412f, .33170732f, .34266611f,
      .34395774f, .34411062f, .34412872f, .34413087f, .34413112f,
      .34413115f };
      linear_solve(L, f);
    }
    void digital_filter2D(std::vector<std::vector<glm::vec4>>& f) const override {
      // Pre-factored L U decomposition of digital filter
      const std::array<float, 9> L{ .23529412f, .33170732f, .34266611f,
      .34395774f, .34411062f, .34412872f, .34413087f, .34413112f,
      .34413115f };

      // for each row
      for (int i = 0; i < f.size(); i++)
        linear_solve(L, f[i]);

      // for each column
      for (int i = 0; i < f[0].size(); i++)
      {
        std::vector<glm::vec4> col;
        for (int r = 0; r < f.size(); r++)
          col.push_back(f[r][i]);
        linear_solve(L, col);
        for (int r = 0; r < f.size(); r++)
          f[r][i] = col[r];
      }
    }

    std::string name() const override { return "Cardinal Cubic OMOMS"; }
    float integral() const override { return 5.25f; }
  };
};

#endif