#ifndef IMAGE_FILTERS_KERNEL_BASE_H
#define IMAGE_FILTERS_KERNEL_BASE_H

#include <cmath>     // std::abs(), std::ceil(), std::floor()
#include <algorithm> // std::min()
#include <vector>    // std::vector<>
#include <array>     // std::array<>
#include <string>    // std::string
#include <cassert>   // assert()

namespace vis
{
  // Kernel interface
  template<size_t N, typename T>
  class KernelBase {
  public:
    KernelBase ()
    {
      b.fill(T(0.f));
    }
    
    // Evaluate kernel at coordinate x
    virtual float operator() (float x) const = 0;
    
    // Apply the kernel’s associated digital filter to sequence f
    virtual void digital_filter (std::vector<T>& f) const = 0;

    // Apply the kernel’s associated digital filter to sequence f
    virtual void digital_filter2D (std::vector<std::vector<T>>& f) const = 0;

    // Kernel support (-support/2, support/2]
    int support () const { return N; }
    
    // Shift the incremental buffer
    float shift_buffer(float a) {
      rotate(b.begin(), b.begin() + 1, b.end());
      std::swap(b.back(), a);
      return a;
    }

    // Incrementally accumulate a sample into buffer
    virtual void accumulate_buffer (float fu, float u) = 0;
    
    // Incrementally reconstruct the function from samples in buffer
    virtual T sample_buffer (float u) const = 0;
    
    virtual std::string name () const = 0;
    
    // How much does the kernel integrate to?
    virtual float integral () const { return 1.f; }

  protected:
    std::array<T, N> b; // Incremental buffer
  };
};

#endif