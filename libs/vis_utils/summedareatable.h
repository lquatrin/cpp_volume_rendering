//http://www.nvidia.com/docs/io/8227/gdc2003_summedareatables.pdf
//https://www.opengl.org/discussion_boards/showthread.php/174243-Summed-Area-Tables-in-GLSL
//https://developer.amd.com/wordpress/media/2012/10/SATsketch-siggraph05.pdf
//http://w3.impa.br/~diego/projects/NehEtAl11/ e http://hhoppe.com/recursive.pdf
//http://www.seas.upenn.edu/~cis565/Lectures2011/Lecture15_SAT.pdf
//http://amd-dev.wpengine.netdna-cdn.com/wordpress/media/2012/10/GDC2005_SATEnvironmentReflections.pdf
//http://www.florian-oeser.de/wordpress/wp-content/2012/10/crow-1984.pdf

//Useful Links:
//  http://stackoverflow.com/questions/20445084/3d-variant-for-summed-area-table-sat

/*
http://apprize.info/programming/opengl_1/12.html
http://www.sunsetlakesoftware.com/2013/10/21/optimizing-gaussian-blurs-mobile-gpu

Texture clamping behaviors:
– Clamp to border color using vec4(0)
– Clamp to Edge using black border around the texture

Special attention: elements out of the upper or left boundary are assumed to evaluate to zero, while elements
out of the bottom or right boundary should be redirected back to the closest element at the respective
boundary (analogous to the clamp-to-edge mode in OpenGL). Bilinear filtering can also be used to sample
the SAT at non-integer locations if necessary.
*/

#ifndef VIS_UTILS_SUMMED_AREA_TABLE_H
#define VIS_UTILS_SUMMED_AREA_TABLE_H

#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <cmath>

#include <gl_utils/texture2d.h>
#include <gl_utils/texture3d.h>

// Using OpenMP to increase the speed of SAT computation
#define USE_OMP
#include <omp.h>

namespace vis
{
  template<typename T>
  class SummedAreaTable2D
  {
  public:
    SummedAreaTable2D (unsigned int _w, unsigned int _h)
      : w(_w), h(_h)
    {
      data = new T[w*h];
      zero = T(0);

      for (int x = 0; x < w; x++)
        data[x] = zero;
      for (int y = 0; y < h; y++)
        data[w*y] = zero;
    }
  
    ~SummedAreaTable2D ()
    {
      if (data)
        delete[] data;
      data = NULL;
    }

    T* GetData()
    {
      return data;
    }
  
    void SetValue (T val, int x, int y)
    {
      data[x + w*y] = val;
    }
  
    T GetValue (int x, int y)
    {
      if (x < 0 || y < 0) return zero;
  
      if (x >= w) x = w - 1;
      if (y >= h) y = h - 1;
  
      return data[x + w*y];
    }
  
    T GetAverage ()
    {
      T v_avg = T(0);
      T nsize = T(w)*T(h);
      for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++)
          v_avg += GetValue(x, y) / nsize;
      return v_avg;
    }
  
    void AddOnEachValue (T v)
    {
      for(int x = 0; x < w; x++)
      {
        for (int y = 0; y < h; y++)
        {
          data[x + w*y] += v;
        }
      }
    }
  
    void GetSizes (int* sw, int* sh)
    {
      *sw = w;
      *sh = h;
    }
  
    T Query (unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1)
    {
      assert(x1 >= x0 && y1 >= y0);
  
      x0 = x0 - 1;
      y0 = y0 - 1;
  
      T XR_YT = GetValue(x1, y1);
      T XL_YB = GetValue(x0, y0);
  
      T XL_YT = GetValue(x0, y1);
      T XR_YB = GetValue(x1, y0);
  
      return XR_YT - XR_YB - XL_YT + XL_YB;
    }
  
    T EvaluateSum (unsigned int x0, unsigned int y0,
                   unsigned int x1, unsigned int y1)
    {
      T sumv = zero;
      for (int x = x0; x <= x1; x++) {
        for (int y = y0; y <= y1; y++) {
          sumv += GetValue(x, y);
        }
      }
  
      return sumv;
    }
  
    // 2010 - Real-time ambient occlusion and halos with Summed Area Tables
    virtual void BuildSAT ()
    {
      //First Step
      SetValue(GetValue(0, 0), 0, 0);
  
      //Second Step
      for (int x = 1; x < w; x++)
        SetValue(GetValue(x - 1, 0) + GetValue(x, 0), x, 0);
      for (int y = 1; y < h; y++)
        SetValue(GetValue(0, y - 1) + GetValue(0, y), 0, y);
  
      //Third Step
      for (int x = 1; x < w; x++)
        for (int y = 1; y < h; y++)
          SetValue(GetValue(x, y)
                 + GetValue(x - 1, y)
                 + GetValue(x, y - 1)
                 - GetValue(x - 1, y - 1)
                 , x, y);
    }
  
    unsigned int w, h;
  protected:
    T* data;
    T zero;
  private:
  };
  
  template<typename T>
  class SummedAreaTable3D
  {
  public:
    SummedAreaTable3D (unsigned int _w, unsigned int _h, unsigned int _d)
      : w(_w), h(_h), d(_d)
    {
      data = new T[w*h*d];
      zero = T(0);
  
      for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++)
          for (int z = 0; z < d; z++)
            SetValue((T)zero, x, y, z);
    }
  
    ~SummedAreaTable3D ()
    {
      if (data)
        delete[] data;
      data = NULL;
    }
  
    T* GetData ()
    {
      return data;
    }

    void SetValue (T val, int x, int y, int z)
    {
      data[x + (w * y) + (w * h * z)] = val;
    }
  
    T GetValue (int x, int y, int z)
    {
      if (x < 0 || y < 0 || z < 0) return zero;
  
      if (x >= w) x = w - 1;
      if (y >= h) y = h - 1;
      if (z >= d) z = d - 1;
  
      return data[x + (w * y) + (w * h * z)];
    }
    
    // 2010 - Real-time ambient occlusion and halos with Summed Area Tables
    // TODO:
    // . Use #pragma omp parallel for
    virtual void BuildSAT ()
    {
      //////////////////////////////////////////////////////////////
      // 1 - First Step
      SetValue(GetValue(0, 0, 0), 0, 0, 0);
  
      //////////////////////////////////////////////////////////////
      // 2 - Second Step
      for (int x = 1; x < w; x++)
        SetValue(GetValue(x - 1, 0, 0) + GetValue(x, 0, 0), x, 0, 0);
      for (int y = 1; y < h; y++)
        SetValue(GetValue(0, y - 1, 0) + GetValue(0, y, 0), 0, y, 0);
      for (int z = 1; z < d; z++)
        SetValue(GetValue(0, 0, z - 1) + GetValue(0, 0, z), 0, 0, z);
  
      //////////////////////////////////////////////////////////////
      // 3 - Third Step
      for (int x = 1; x < w; x++)
        for (int z = 1; z < d; z++)
          SetValue(GetValue(x - 1, 0, z) +
                   GetValue(x, 0, z - 1) -
                   GetValue(x - 1, 0, z - 1) +
                   GetValue(x, 0, z),
                   x, 0, z);
      for (int x = 1; x < w; x++)
        for (int y = 1; y < h; y++)
          SetValue(GetValue(x - 1, y, 0) +
                   GetValue(x, y - 1, 0) - 
                   GetValue(x - 1, y - 1, 0) +
                   GetValue(x, y, 0),
                   x, y, 0);
      for (int y = 1; y < h; y++)
        for (int z = 1; z < d; z++)
          SetValue(GetValue(0, y - 1, z) + 
                   GetValue(0, y, z - 1) - 
                   GetValue(0, y - 1, z - 1) + 
                   GetValue(0, y, z),
                   0, y, z);

      //////////////////////////////////////////////////////////////
      // 4 - Fourth Step
      for (int x = 1; x < w; x++)
      {
        for (int y = 1; y < h; y++)
        {
          for (int z = 1; z < d; z++)
          {
            T val = GetValue(x, y, z)
                  + GetValue(x - 1, y - 1, z - 1)
                  + GetValue(x, y, z - 1)
                  + GetValue(x, y - 1, z)
                  + GetValue(x - 1, y, z)
                  - GetValue(x - 1, y - 1, z)
                  - GetValue(x, y - 1, z - 1)
                  - GetValue(x - 1, y, z - 1);
  
            SetValue(val, x, y, z);
          }
        }
      }
    }
  
    T GetAverage ()
    {
      T avgvl = T(0);
      T nsize = T(w)*T(h)*T(d);
      for (int x = 0; x < w; x++)
      {
        for (int y = 0; y < h; y++)
        {
          for (int z = 0; z < d; z++)
          {
            avgvl += GetValue(x, y, z) / nsize;
          }
        }
      }
      return avgvl;
    }

    unsigned int w, h, d;
  protected:
    T* data;
    T zero;

  private:
  };
}

#endif