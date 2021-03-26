#ifndef IMAGE_FILTERS
#define IMAGE_FILTERS

#include "kernelbase.hpp"

namespace vis
{
  enum IMAGE_FILTER_KERNEL : unsigned int {
    K1_BOX                = 0,
    K2_HAT                = 1,
    K4_CATMULL_ROM        = 2,
    K4_MITCHELL_NETRAVALI = 3,
    K4_CARDINAL_BSPLINE_3 = 4,
    K4_CARDINAL_OMOMS3    = 5,
  };

  // Apply to sequence f the inverse discrete convolution given by
  // a pre-factored LU decomposition
  template<size_t M, typename T>
  void linear_solve(const std::array<float, M>& L, std::vector<T>& f)
  {
    const int m = M, n = int(f.size());
    const float p_inv = 1.f; // Optimized for prescaled kernel where p = 1
    // Pre-factored decomposition only works when n>m. Grow sequence f if needed.
    while (f.size() <= m)
    {
      f.reserve(2 * f.size()); // Prevent reallocation during insertions
      f.insert(f.end(), f.rbegin(), f.rend()); // Append a reflection
    }

    int nn = int(f.size()); // New size
    const float L_inf = L[m - 1], v_inv = L_inf / (1.f + L_inf);

    // Forward pass: solve Lc0 = f in-place
    for (int i = 1; i < m; i++) f[i] -= L[i - 1] * f[i - 1];
    for (int i = m; i < nn; i++) f[i] -= L_inf * f[i - 1];

    // Reverse pass: solve Uc = c0 in-place
    f[nn - 1] *= p_inv * v_inv;
    for (int i = nn - 2; i >= m - 1; i--) f[i] = L_inf * (p_inv * f[i] - f[i + 1]);
    for (int i = m - 2; i >= 0; i--) f[i] = L[i] * (p_inv * f[i] - f[i + 1]);
    f.resize(n); // Truncate back to original size if grown
  }

  // Most cubics are C1-continuous, symmetric, and have support 4.
  // Factor out common functionality into a class.
  template<typename Pieces, typename T>
  class Symmetric4Pieces : public KernelBase<4, T>
  {
  public:
    float operator() (float x) const override final {
      x = abs(x);
      return x > 2.0f ? 0.0f : x > 1.0f ? p.k0(2.0f - x) : p.k1(1.0f - x);
    }
    void accumulate_buffer(float fu, float u) override final {
      b[0] += fu * p.k3(u);
      b[1] += fu * p.k2(u);
      b[2] += fu * p.k1(u);
      b[3] += fu * p.k0(u);
    }
    T sample_buffer(float u) const override final {
      return b[0] * p.k3(u) + b[1] * p.k2(u) + b[2] * p.k1(u) + b[3] * p.k0(u);
    }
  private:
    Pieces p; // Polynomial pieces of kernel (k0:[-2,-1], k1:[-1,0], k2:[0,1], k3:[1,2]
  };
};

#endif