#include "utils.h"

#include <vis_utils/summedareatable.h>
#include <iostream>
#include <random>
#include <fstream>

#define TEXTURE_FILTER GL_LINEAR        // GL_NEAREST         //
#define TEXTURE_WRAP   GL_CLAMP_TO_EDGE // GL_CLAMP_TO_BORDER // 

namespace vis
{
  struct GLFloat4 {
    GLfloat r;
    GLfloat g;
    GLfloat b;
    GLfloat a;
  };

  gl::Texture3D* GenerateRTexture(StructuredGridVolume* vol, int init_x, int init_y, int init_z,
    int last_x, int last_y, int last_z)
  {
    if (!vol) return NULL;

    int size_x = abs(last_x - init_x);
    int size_y = abs(last_y - init_y);
    int size_z = abs(last_z - init_z);

    GLfloat* scalar_values = new GLfloat[size_x*size_y*size_z];

    for (int k = 0; k < size_z; k++)
    {
      for (int j = 0; j < size_y; j++)
      {
        for (int i = 0; i < size_x; i++)
        {
          scalar_values[i + (j * size_x) + (k * size_x * size_y)] = (GLfloat)vol->GetNormalizedSample((i + init_x), (j + init_y), (k + init_z));
        }
      }
    }

    gl::Texture3D* tex3d_r = new gl::Texture3D(size_x, size_y, size_z);

    tex3d_r->GenerateTexture(TEXTURE_FILTER, TEXTURE_FILTER, TEXTURE_WRAP, TEXTURE_WRAP, TEXTURE_WRAP);

#ifdef USE_16F_INTERNAL_FORMAT
    tex3d_r->SetData(scalar_values, GL_R16F, GL_RED, GL_FLOAT);
#else
    tex3d_r->SetData(scalar_values, GL_R32F, GL_RED, GL_FLOAT);
#endif
    gl::ExitOnGLError("ERROR: After SetData");

    delete[] scalar_values;

    return tex3d_r;
  }

  gl::Texture3D* GenerateRTexture (StructuredGridVolume* vol, VIS_UTILS_DATA_TYPE vdatatype)
  {
    if (!vol) return NULL;

    int size_x = vol->GetWidth();
    int size_y = vol->GetHeight();
    int size_z = vol->GetDepth();
    
    gl::Texture3D* tex3d_r = new gl::Texture3D(size_x, size_y, size_z);

    if (vdatatype == VIS_UTILS_DATA_TYPE::UNSIGNED_BYTE)
    {
      GLubyte* scalar_values = new GLubyte[size_x*size_y*size_z];
      
      for (int k = 0; k < size_z; k++)
      {
        for (int j = 0; j < size_y; j++)
        {
          for (int i = 0; i < size_x; i++)
          {
            scalar_values[i + (j * size_x) + (k * size_x * size_y)] = (GLubyte)((vol->GetNormalizedSample(i, j, k)) * 255.0);
          }
        }
      }
      tex3d_r->GenerateTexture(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      tex3d_r->SetData(scalar_values, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE);
      delete[] scalar_values;
    }
    else if (vdatatype == VIS_UTILS_DATA_TYPE::UNSIGNED_SHORT)
    {
      GLushort* scalar_values = new GLushort[size_x*size_y*size_z];

      for (int k = 0; k < size_z; k++)
      {
        for (int j = 0; j < size_y; j++)
        {
          for (int i = 0; i < size_x; i++)
          {
            scalar_values[i + (j * size_x) + (k * size_x * size_y)] = (GLushort)((vol->GetNormalizedSample(i, j, k)) * 65535.0);
          }
        }
      }
      tex3d_r->GenerateTexture(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      tex3d_r->SetData(scalar_values, GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT);
      delete[] scalar_values;
    }
    else if (vdatatype == VIS_UTILS_DATA_TYPE::HALF_FLOAT)
    {
      GLfloat* scalar_values = new GLfloat[size_x*size_y*size_z];

      for (int k = 0; k < size_z; k++)
      {
        for (int j = 0; j < size_y; j++)
        {
          for (int i = 0; i < size_x; i++)
          {
            scalar_values[i + (j * size_x) + (k * size_x * size_y)] = (GLfloat)(vol->GetNormalizedSample(i, j, k));
          }
        }
      }
      tex3d_r->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      tex3d_r->SetData(scalar_values, GL_R16F, GL_RED, GL_FLOAT);
      delete[] scalar_values;
    }
    else if (vdatatype == VIS_UTILS_DATA_TYPE::FLOAT)
    {
      GLfloat* scalar_values = new GLfloat[size_x*size_y*size_z];

      for (int k = 0; k < size_z; k++)
      {
        for (int j = 0; j < size_y; j++)
        {
          for (int i = 0; i < size_x; i++)
          {
            scalar_values[i + (j * size_x) + (k * size_x * size_y)] = (GLfloat)(vol->GetNormalizedSample(i, j, k));
          }
        }
      }
      tex3d_r->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      tex3d_r->SetData(scalar_values, GL_R32F, GL_RED, GL_FLOAT);
      delete[] scalar_values;
    }

    gl::ExitOnGLError("ERROR: After SetData");

    return tex3d_r;
  }

  gl::Texture3D* GenerateGradientTexture(StructuredGridVolume* vol, int gradient_sample_size,
    int filter_nxnxn, bool normalized_gradient,
    int init_x, int init_y, int init_z,
    int last_x, int last_y, int last_z)
  {
    int width = vol->GetWidth();
    int height = vol->GetHeight();
    int depth = vol->GetDepth();

    //1
    //Generation of gradients
    int n = gradient_sample_size;
    glm::dvec3* gradients = new glm::dvec3[width * height * depth];
    glm::dvec3 s1, s2;
    int index = 0;
    for (int z = 0; z < depth; z++)
    {
      for (int y = 0; y < height; y++)
      {
        for (int x = 0; x < width; x++)
        {
          s1.x = vol->GetNormalizedSample(x - n, y, z);
          s2.x = vol->GetNormalizedSample(x + n, y, z);
          s1.y = vol->GetNormalizedSample(x, y - n, z);
          s2.y = vol->GetNormalizedSample(x, y + n, z);
          s1.z = vol->GetNormalizedSample(x, y, z - n);
          s2.z = vol->GetNormalizedSample(x, y, z + n);

          glm::dvec3 s2s1 = (s2 - s1);

          if (normalized_gradient)
          {
            s2s1 = glm::normalize<double>(s2s1);
          }
          else
          {
            s2s1.x = s2s1.x / 2.0f * (float)gradient_sample_size;
            s2s1.y = s2s1.y / 2.0f * (float)gradient_sample_size;
            s2s1.z = s2s1.z / 2.0f * (float)gradient_sample_size;
          }

          gradients[index] = s2s1;

          if (gradients[index].x != gradients[index].x) //lm.IsNaN
            gradients[index] = glm::dvec3(0);

          index++;
        }
      }
    }

    //2
    //Filtering
    n = filter_nxnxn;
    index = 0;
    if (n > 0)
    {
      for (int z = 0; z < depth; z++)
      {
        for (int y = 0; y < height; y++)
        {
          for (int x = 0; x < width; x++)
          {
            int fn = (n - 1) / 2;

            glm::dvec3 average = glm::dvec3(0);
            int num = 0;
            for (int k = z - fn; k <= z + fn; k++)
            {
              for (int j = y - fn; j <= y + fn; j++)
              {
                for (int i = x - fn; i <= x + fn; i++)
                {
                  if (!vol->IsOutOfBoundary(i, j, k))
                  {
                    average += gradients[x + (y * width) + (z * width * height)];
                    num++;
                  }
                }
              }
            }

            average = average / (double)num;
            if (average.x != 0.0f && average.y != 0.0f && average.z != 0.0f)
              average = glm::normalize<double>(average);

            gradients[index++] = average;
          }
        }
      }
    }

    //3
    //Set the content of the gradient texture
    if (init_x == -1 && last_x == -1
      && init_y == -1 && last_y == -1
      && init_z == -1 && last_z == -1)
    {
      init_x = 0;
      init_y = 0;
      init_z = 0;

      last_x = vol->GetWidth();
      last_y = vol->GetHeight();
      last_z = vol->GetDepth();
    }

    int size_x = abs(last_x - init_x);
    int size_y = abs(last_y - init_y);
    int size_z = abs(last_z - init_z);
    glm::vec3* gradients_values = new glm::vec3[size_x*size_y*size_z];

    for (int k = 0; k < size_z; k++)
    {
      for (int j = 0; j < size_y; j++)
      {
        for (int i = 0; i < size_x; i++)
        {
          gradients_values[i + (j * size_x) + (k * size_x * size_y)] = gradients[(i + init_x) + ((j + init_y) * width) + ((k + init_z) * width * height)];
        }
      }
    }

    //4
    //Creating Texture
    gl::Texture3D* tex3d_gradient = new gl::Texture3D(size_x, size_y, size_z);
    tex3d_gradient->GenerateTexture(TEXTURE_FILTER, TEXTURE_FILTER, TEXTURE_WRAP, TEXTURE_WRAP, TEXTURE_WRAP);

#ifdef USE_16F_INTERNAL_FORMAT
    tex3d_gradient->SetData((GLvoid*)gradients_values, GL_RGB16F, GL_RGB, GL_FLOAT);
#else
    tex3d_gradient->SetData((GLvoid*)gradients_values, GL_RGB32F, GL_RGB, GL_FLOAT);
#endif

    delete[] gradients_values;
    delete[] gradients;

    return tex3d_gradient;
  }

  // https://en.wikipedia.org/wiki/Sobel_operator  
  gl::Texture3D* GenerateSobelFeldmanGradientTexture(StructuredGridVolume* vol)
  {
    int width = vol->GetWidth();
    int height = vol->GetHeight();
    int depth = vol->GetDepth();

    int n = 1;
    glm::dvec3* gradients = new glm::dvec3[width * height * depth];
    for (int z = 0; z < depth; z++)
    {
      for (int y = 0; y < height; y++)
      {
        for (int x = 0; x < width; x++)
        {
          glm::dvec3 sg(0.0);
          for (int v1 = -1; v1 <= 1; v1++)
          {
            for (int v2 = -1; v2 <= 1; v2++)
            {
              sg.z += ((double)vol->GetNormalizedSample(x + v1, y + v2, z - 1)) * (4.0 / pow(2.0, glm::abs(v1) + glm::abs(v2)))
                + ((double)vol->GetNormalizedSample(x + v1, y + v2, z + 1)) * (-4.0 / pow(2.0, glm::abs(v1) + glm::abs(v2)));

              sg.y += ((double)vol->GetNormalizedSample(x + v1, y - 1, z + v2)) * (4.0 / pow(2.0, glm::abs(v1) + glm::abs(v2)))
                + ((double)vol->GetNormalizedSample(x + v1, y + 1, z + v2)) * (-4.0 / pow(2.0, glm::abs(v1) + glm::abs(v2)));

              sg.x += ((double)vol->GetNormalizedSample(x - 1, y + v2, z + v1)) * (4.0 / pow(2.0, glm::abs(v1) + glm::abs(v2)))
                + ((double)vol->GetNormalizedSample(x + 1, y + v2, z + v1)) * (-4.0 / pow(2.0, glm::abs(v1) + glm::abs(v2)));
            }
          }

          // not normalized (for tests...)
          gradients[x + (y * width) + (z * width * height)] = sg;
        }
      }
    }

    glm::vec3* gradients_values = new glm::vec3[width * height * depth];
    for (int k = 0; k < depth; k++)
    {
      for (int j = 0; j < height; j++)
      {
        for (int i = 0; i < width; i++)
        {
          gradients_values[i + (j * width) + (k * width * height)] = gradients[i + (j * width) + (k * width * height)];
        }
      }
    }

    //4
    //Creating Texture
    gl::Texture3D* tex3d_gradient = new gl::Texture3D(width, height, depth);
    tex3d_gradient->GenerateTexture(TEXTURE_FILTER, TEXTURE_FILTER, TEXTURE_WRAP, TEXTURE_WRAP, TEXTURE_WRAP);

#ifdef USE_16F_INTERNAL_FORMAT
    tex3d_gradient->SetData((GLvoid*)gradients_values, GL_RGB16F, GL_RGB, GL_FLOAT);
#else
    tex3d_gradient->SetData((GLvoid*)gradients_values, GL_RGB32F, GL_RGB, GL_FLOAT);
#endif

    delete[] gradients_values;
    delete[] gradients;

    return tex3d_gradient;
  }

  gl::Texture2D* GenerateNoiseTexture(float maxvalue, int w, int h)
  {
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

    GLfloat* disturb_points = new GLfloat[w * h];
    for (int i = 0; i < w * h; i++) {
      float number = distribution(generator) * maxvalue;
      disturb_points[i] = number;
    }

    gl::Texture2D* tex2d = new gl::Texture2D(w, h);
    tex2d->GenerateTexture(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
#ifdef USE_16F_INTERNAL_FORMAT
    tex2d->SetData(disturb_points, GL_R16F, GL_RED, GL_FLOAT);
#else
    tex2d->SetData(disturb_points, GL_R32F, GL_RED, GL_FLOAT);
#endif
    return tex2d;
  }

  void GenerateSyntheticVolumetricModels(int d, float s)
  {
    std::ofstream gaussianvol_file;
    gaussianvol_file.open("synthetic_gaussianvol_file.syn");
    if (gaussianvol_file.is_open())
    {
      gaussianvol_file << d << " " << d << " " << d << "\n";
      for (int x = 0; x < d; x++)
      {
        for (int y = 0; y < d; y++)
        {
          for (int z = 0; z < d; z++)
          {
            float dx = x - (d / 2);
            float dy = y - (d / 2);
            float dz = z - (d / 2);
            float val = glm::exp(-(dx*dx + dy * dy + dz * dz) / (2.0f * s * s));
            gaussianvol_file << "0 " << x << " " << y << " " << z << " " << int(val * 255.0f) << "\n";
          }
        }
      }
      gaussianvol_file.close();
    }
  }

  gl::Texture3D* GenerateExtinctionSAT3DTex(StructuredGridVolume* vol, TransferFunction* tf)
  {
    // 1
    // First, sample the initial "grid" and build SAT
    vis::SummedAreaTable3D<double> sat3d(vol->GetWidth(), vol->GetHeight(), vol->GetDepth());
    for (int x = 0; x < vol->GetWidth(); x++)
    {
      for (int y = 0; y < vol->GetHeight(); y++)
      {
        for (int z = 0; z < vol->GetDepth(); z++)
        {
          double val = tf->GetExt(vol->GetNormalizedSample(x, y, z), true);
          sat3d.SetValue(val, x, y, z);
        }
      }
    }
    sat3d.BuildSAT();

    // 2
    // Then, we must create and generate the 3D texture
    GLfloat* diff_mat = new GLfloat[vol->GetWidth() * vol->GetHeight() * vol->GetDepth()];
    double* sat_data = sat3d.GetData();

    //*
    for (int i = 0; i < vol->GetWidth() * vol->GetHeight() * vol->GetDepth(); i++)
      diff_mat[i] = (GLfloat)sat_data[i];
    // */

    /*
    for (int x = 0; x < vol->GetWidth(); x++)
    {
      for (int y = 0; y < vol->GetHeight(); y++)
      {
        for (int z = 0; z < vol->GetDepth(); z++)
        {
          int i = x + (y * vol->GetWidth()) + (z * vol->GetWidth() * vol->GetHeight());
          double sd = sat_data[i] / double((x+1)*(y+1)*(z+1));
          diff_mat[i] = sd;
        }
      }
    }
    // */

    gl::Texture3D* tex3d_sat = new gl::Texture3D(vol->GetWidth(), vol->GetHeight(), vol->GetDepth());
    tex3d_sat->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

#ifdef USE_16F_INTERNAL_FORMAT
    tex3d_sat->SetData((GLvoid*)diff_mat, GL_R16F, GL_RED, GL_FLOAT);
#else
    tex3d_sat->SetData((GLvoid*)diff_mat, GL_R32F, GL_RED, GL_FLOAT);
#endif

    delete[] diff_mat;

    gl::ExitOnGLError("volrend/utils.cpp - GenerateExtinctionSAT3DTex()");
    return tex3d_sat;
  }

  gl::Texture3D* GenerateScalarFieldSAT3DTex (StructuredGridVolume* vol)
  {
    // 1
    // First, sample the initial "grid" and build SAT
    vis::SummedAreaTable3D<double> sat3d(vol->GetWidth(), vol->GetHeight(), vol->GetDepth());
    for (int x = 0; x < vol->GetWidth(); x++)
    {
      for (int y = 0; y < vol->GetHeight(); y++)
      {
        for (int z = 0; z < vol->GetDepth(); z++)
        {
          double val = (double)vol->GetNormalizedSample(x, y, z);
          sat3d.SetValue(val, x, y, z);
        }
      }
    }
    sat3d.BuildSAT();

    // 2
    // Then, we must create and generate the 3D texture
    GLfloat* diff_mat = new GLfloat[vol->GetWidth() * vol->GetHeight() * vol->GetDepth()];
    double* sat_data = sat3d.GetData();
    for (int i = 0; i < vol->GetWidth() * vol->GetHeight() * vol->GetDepth(); i++)
      diff_mat[i] = (GLfloat)sat_data[i];

    gl::Texture3D* tex3d_sat = new gl::Texture3D(vol->GetWidth(), vol->GetHeight(), vol->GetDepth());
    tex3d_sat->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    tex3d_sat->SetData((GLvoid*)diff_mat, GL_R32F, GL_RED, GL_FLOAT);

    delete[] diff_mat;

    gl::ExitOnGLError("volrend/utils.cpp - GenerateScalarFieldSAT3DTex()");
    return tex3d_sat;

  }
}