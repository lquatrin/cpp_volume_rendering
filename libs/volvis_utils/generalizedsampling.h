// Encapsulated codes from "A Fresh Look at Generalized Sampling"
//
// http://w3.impa.br/~diego/publications/NehHop14.pdf
// https://www.nowpublishers.com/article/Details/CGV-053
#ifndef A_FRESH_LOOK_AT_GENERALIZED_SAMPLING
#define A_FRESH_LOOK_AT_GENERALIZED_SAMPLING

#include <vector>
#include <vis_utils/filters/kernelbase.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <iostream>  // std::ostream, std::cout
#include <cmath>     // std::abs(), std::ceil(), std::floor()
#include <algorithm> // std::min()
#include <vector>    // std::vector<>
#include <array>     // std::array<>
#include <string>    // std::string
#include <cassert>   // assert()
#include <chrono>    // std::chrono for timing benchmarks
#include <iomanip>   // std::setprecision

namespace vis
{
//#define USE_INCREMENTAL_IMPLEMENTATION

// Output a sequence
  static std::ostream& operator<<(std::ostream& out, const std::vector<std::vector<float>>& f) {
    for (int i = 0; i < f.size(); i++)
    {
      //out << "(" << (i + .5) / f.size() << ", " << float(f[i]) << ")" << " ";
      for (int j = 0; j < f[i].size(); j++)
      {
        out << std::setprecision(4) << f[i][j] << " ";
      }
      out << '\n';
    }
    out << '\n';
    return out;
  }
  
class GeneralizedSampling
{
private:
  // Given sequence f, access f[i] using reflection at boundaries
  static inline float get(const std::vector<float>& f, int i) {
    return i < 0 ? get(f, -i - 1) : i >= int(f.size()) ?
      get(f, 2 * int(f.size()) - i - 1) : f[i];
  }
  // Given sequence f, access f[i] using reflection at boundaries
  static inline glm::vec4 get(const std::vector<glm::vec4>& f, int i) {
    return i < 0 ? get(f, -i - 1) : i >= int(f.size()) ?
      get(f, 2 * int(f.size()) - i - 1) : f[i];
  }
  // Given sequence f, access f[i] using reflection at boundaries
  static inline float get(const std::vector<std::vector<float>>& m, int r, int c) {
    if (r < 0) {
      return get(m, -r - 1, c);
    }
    else {
      if (r >= m.size()) {
        return get(m, 2 * int(m.size()) - r - 1, c);
      }
    }

    if (c < 0) {
      return get(m, r, -c - 1);
    }
    else {
      if (c >= m[0].size()) {
        return get(m, r, 2 * int(m[0].size()) - c - 1);
      }
    }

    return m[r][c];
  }
  // Given sequence f, access f[i] using reflection at boundaries
  static inline glm::vec4 get(const std::vector<std::vector<glm::vec4>>& m, int r, int c) {
    if (r < 0) {
      return get(m, -r - 1, c);
    }
    else {
      if (r >= m.size()) {
        return get(m, 2 * int(m.size()) - r - 1, c);
      }
    }

    if (c < 0) {
      return get(m, r, -c - 1);
    }
    else {
      if (c >= m[0].size()) {
        return get(m, r, 2 * int(m[0].size()) - c - 1);
      }
    }

    return m[r][c];
  }
  // Advance to next sample
  inline bool advance(int& i, int& j, double& u, double inv_s) {
    ++i; u += inv_s;
    if (u < 1.) return false;
    u -= 1.; ++j; return true;
  }

public:
  GeneralizedSampling ();
  ~GeneralizedSampling ();

  template<typename Kernel>
  std::vector<float> IntuitiveDownSample (std::vector<float> f, int m, Kernel& k) {
    assert(m <= int(f.size())); // Ensure we are downsampling
    float s = float(m) / f.size(); // Scale factor
    const int n = int(f.size());
    const float kr = .5f * float(k.support());
    const bool should_normalize = (f.size() % m != 0);
    std::vector<float> g(m); // New sequence of desired size m<f.size()
    for (int j = 0; j < m; j++) { // Index of sample in g
      float x = (j + .5f) / m; // Position in domain [0,1] of both f and g
      int il = int(ceil((x - kr / m) * n - .5f)); // Leftmost sample under kernel
      int ir = int(floor((x + kr / m) * n - .5f)); // Rightmost sample under kernel
      if (should_normalize) { // Should normalize?
        double sum = 0., sumw = 0.; // Sums of values and weights
        for (int i = il; i <= ir; i++) { // Loop over input samples
          float w = k((x - (i + .5f) / n) * m); // Weight for sample
          sum += w * get(f, i); sumw += w; // Accumulate values and weights
        }
        g[j] = k.integral() * float(sum / sumw); // Normalize by summed weights
      }
      else {
        for (int i = il; i <= ir; i++) {
          g[j] += k((x - (i + .5f) / n) * m) * get(f, i);
        }
        g[j] *= s;
      }
    }
    k.digital_filter(g); // Apply kernel’s associated digital filter
    return g;
  }
  template<typename Kernel>
  std::vector<float> IncrementalDownSample (std::vector<float> f, int m, Kernel& k) {
    const int n = f.size();
    double s = double(m) / n; // Scale factor
    double kr = .5 * k.support();
    int fi = int(ceil(((.5 - kr) / m) * n - .5));
    // Input sample position between output samples
    double u = ((fi + .5) / n) * m - (.5 - kr);
    assert(f.size() >= m); // Ensure we are downsampling
    std::vector<float> g(m); // New sequence of desired size m>f.size()
    int gi = -k.support();
    for (int i = 0; i < k.support(); i++) // Initialize incremental buffer
      k.shift_buffer(0.f);
    while (gi < -1) {
      k.accumulate_buffer(get(f, fi), u);
      if (advance(fi, gi, u, s)) k.shift_buffer(0.f);
    }
    while (1) { // Accumulate weighted f samples into g
      k.accumulate_buffer(get(f, fi), u);
      if (advance(fi, gi, u, s)) {
        if (gi >= m) break;
        g[gi] = s * k.shift_buffer(0.f);
      }
    }
    k.digital_filter(g);
    return g;
  }
  template<typename Kernel>
  std::vector<float> IntuitiveUpSample (std::vector<float> f, int m, Kernel& k) {
    assert(m >= int(f.size())); // Ensure we are upsampling
    std::vector<float> g(m); // New sequence of desired size m>f.size()
    k.digital_filter(f); // Apply kernel’s associated digital filter
    const float kr = .5f * float(k.support());
    for (int j = 0; j < m; j++) { // Index of sample in g
      float x = (j + .5f) / m; // Position in domain [0,1] of both f and g
      float xi = x * f.size() - .5f; // Position in input sequence f
      int il = int(ceil(xi - kr)); // Leftmost sample under kernel support
      int ir = int(floor(xi + kr)); // Rightmost sample under kernel support
      double sum = 0.;
      for (int i = il; i <= ir; i++)
        sum += get(f, i) * k(xi - i);                              
      g[j] = float(sum);
    }
    return g;
  }
  template<typename Kernel>
  std::vector<float> IncrementalUpSample (std::vector<float> f, int m, Kernel& k) {
    k.digital_filter(f);
    double inv_s = double(f.size()) / m; // Inverse scale factor
    // Output sample position between input samples
    double u = .5 * (inv_s + (k.support() + 1) % 2);
    int fi = -k.support() / 2 - 1;
    assert(f.size() <= m); // Ensure we are upsampling
    std::vector<float> g(m); // New sequence of desired size m>f.size()
    for (int i = 0; i < k.support(); i++) // Initialize incremental buffer
      k.shift_buffer(get(f, ++fi));

    for (int gi = 0; gi < m; ) { // Sample reconstruction of f into g[gi]
      g[gi] = k.sample_buffer(u);
      if (advance(gi, fi, u, inv_s))
        k.shift_buffer(get(f, fi));
    }
    return g;
  }
  template<typename Kernel>
  std::vector<float> ApplyFiltering1D (std::vector<float> f, int m, bool use_incremental) {
    Kernel k;
    if (use_incremental)
    {
      if (m <= int(f.size()))
        return IncrementalDownSample(f, m, k);
      return IncrementalUpSample(f, m, k);
    }

    if (m <= int(f.size()))
      return IntuitiveDownSample(f, m, k);
    return IntuitiveUpSample(f, m, k);
  }

  template<typename Kernel, typename T>
  std::vector<std::vector<T>> IntuitiveDownSample2D (std::vector<std::vector<T>> f, int r, int c, Kernel& k) {
    assert(int(f.size()) % r == 0 && int(f[0].size()) % c == 0); // Ensure is always "normalized"  
    assert(r <= int(f.size()) && c <= int(f[0].size())); // Ensure we are downsampling
    
    float s_r = float(r) / f.size(); // Scale factor
    float s_c = float(c) / f[0].size(); // Scale factor

    const int n_r = int(f.size());
    const int n_c = int(f[0].size());

    const float kr = .5f * float(k.support());
    
    std::vector<std::vector<T>> g(r);
    for (int i = 0; i < r; i++) g[i] = std::vector<T>(c);

    for (int j_r = 0; j_r < r; j_r++) { // Index of sample in g
      float x_r = (j_r + .5f) / r; // Position in domain [0,1] of both f and g
      int il_r = int(ceil((x_r - kr / r) * n_r - .5f)); // Leftmost sample under kernel
      int ir_r = int(floor((x_r + kr / r) * n_r - .5f)); // Rightmost sample under kernel

      for (int j_c = 0; j_c < c; j_c++) { // Index of sample in g
        float x_c = (j_c + .5f) / c; // Position in domain [0,1] of both f and g
        int il_c = int(ceil((x_c - kr / c) * n_c - .5f)); // Leftmost sample under kernel
        int ir_c = int(floor((x_c + kr / c) * n_c - .5f)); // Rightmost sample under kernel

        for (int i_r = il_r; i_r <= ir_r; i_r++) {
          for (int i_c = il_c; i_c <= ir_c; i_c++) {
            T get_value = get(f, i_r, i_c);

            g[j_r][j_c] += (
              k((x_r - (i_r + .5f) / n_r) * r) 
              *
              k((x_c - (i_c + .5f) / n_c) * c)
            ) * get_value;
          }
        }
        g[j_r][j_c] *= (s_r * s_c);
      }
    }
    k.digital_filter2D(g); // Apply kernel’s associated digital filter
   
    return g;
  }
  template<typename Kernel, typename T>
  std::vector<std::vector<T>> IntuitiveUpSample2D (std::vector<std::vector<T>> f, int r, int c, Kernel& k) {
    assert(r % int(f.size()) == 0 && c % int(f[0].size()) == 0); // Ensure is always "normalized"  
    assert(r >= int(f.size()) && c >= int(f[0].size())); // Ensure we are upsampling

    std::vector<std::vector<T>> g(r);
    for (int i = 0; i < r; i++) g[i] = std::vector<T>(c);
    
    k.digital_filter2D(f); // Apply kernel’s associated digital filter
    const float kr = .5f * float(k.support());
    
    for (int j_r = 0; j_r < r; j_r++) { // Index of sample in g
      float x_r = (j_r + .5f) / r; // Position in domain [0,1] of both f and g
      float xi_r = x_r * f.size() - .5f; // Position in input sequence f
      int il_r = int(ceil(xi_r - kr)); // Leftmost sample under kernel support
      int ir_r = int(floor(xi_r + kr)); // Rightmost sample under kernel support

      for (int j_c = 0; j_c < c; j_c++) { // Index of sample in g
        float x_c = (j_c + .5f) / c; // Position in domain [0,1] of both f and g
        float xi_c = x_c * f[0].size() - .5f; // Position in input sequence f
        int il_c = int(ceil(xi_c - kr)); // Leftmost sample under kernel support
        int ir_c = int(floor(xi_c + kr)); // Rightmost sample under kernel support

        T sum = T(0.);
        for (int i_r = il_r; i_r <= ir_r; i_r++)
        {
          for (int i_c = il_c; i_c <= ir_c; i_c++)
          {
            T get_value = get(f, i_r, i_c);
            sum += (
              k(xi_r - i_r) * k(xi_c - i_c)
            ) * get_value;
          }
        }
        g[j_r][j_c] = T(sum);
      }
    }

    return g;
  }
  template<typename Kernel, typename T>
  std::vector<std::vector<T>> ApplyFiltering2D (std::vector<std::vector<T>> f, int r, int c) {
    Kernel k;
    //if (use_incremental)
    //{
    //  if (m <= int(f.size()))
    //    return IncrementalDownSample(f, m, k);
    //  return IncrementalUpSample(f, m, k);
    //}

    if (r <= int(f.size()) && c <= int(f[0].size()))
    {
      return IntuitiveDownSample2D(f, r, c, k);
    }
    else if (r > int(f.size()) && c > int(f[0].size()))
    {
      return IntuitiveUpSample2D(f, r, c, k);
    }
    return f;
  }


protected:

private:
};
};

#endif