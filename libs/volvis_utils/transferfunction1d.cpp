#include "transferfunction1d.h"
#include <gl_utils/texture1d.h>
#include <GL/glew.h>

#include <fstream>
#include <cstdlib>

namespace vis
{
  TransferFunction1D::TransferFunction1D (int max_value)
    : m_built (false)
  {
    max_density = max_value;

    m_cpt_rgb.clear ();
    m_cpt_alpha.clear ();
    m_transferfunction = NULL;

    extinction_coef_type = false;
  }

  TransferFunction1D::~TransferFunction1D ()
  {
    m_cpt_rgb.clear ();
    m_cpt_alpha.clear ();
    if (m_transferfunction)
      m_transferfunction;
    if (m_gradients)
      m_gradients;
  }

  const char* TransferFunction1D::GetNameClass ()
  {
    return "TrasnferFunction1D";
  }

  void TransferFunction1D::SetExtinctionCoefficientInput(bool s)
  {
    extinction_coef_type = s;
  }

  void TransferFunction1D::AddRGBControlPoint (TransferControlPoint rgb)
  {
    m_cpt_rgb.push_back (rgb);
  }

  void TransferFunction1D::AddAlphaControlPoint (TransferControlPoint alpha)
  {
    m_cpt_alpha.push_back (alpha);
  }

  void TransferFunction1D::ClearControlPoints ()
  {
    m_cpt_rgb.clear ();
    m_cpt_alpha.clear ();
  }

  gl::Texture1D* TransferFunction1D::GenerateTexture_1D_RGBA ()
  {
    if (!m_built)
      Build();

    if (m_transferfunction)
    {
      int tf_size = max_density + 1;
      gl::Texture1D* ret = new gl::Texture1D (tf_size);
      ret->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
      float* data = new float[tf_size * 4];
      for (int i = 0; i < tf_size; i++)
      {
        data[(i * 4)]     = (float)(m_transferfunction[i].r);
        data[(i * 4) + 1] = (float)(m_transferfunction[i].g);
        data[(i * 4) + 2] = (float)(m_transferfunction[i].b);

        float v4 = (double)(m_transferfunction[i].a);

        if (extinction_coef_type)
          v4 = ExtinctionToMaterialOpacity(v4);
        
        data[(i * 4) + 3] = v4;
      }
      ret->SetData ((void*)data, GL_RGBA16F, GL_RGBA, GL_FLOAT);
      delete[] data;
      return ret;
    }
    return NULL;
  }

  gl::Texture1D* TransferFunction1D::GenerateTexture_1D_RGBt ()
  {
    if (!m_built)
      Build();

    if (m_transferfunction)
    {
      int tf_size = max_density + 1;
      gl::Texture1D* ret = new gl::Texture1D(tf_size);
      ret->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
      float* data = new float[tf_size * 4];
      for (int i = 0; i < tf_size; i++)
      {
        data[(i * 4)]     = (float)(m_transferfunction[i].r);
        data[(i * 4) + 1] = (float)(m_transferfunction[i].g);
        data[(i * 4) + 2] = (float)(m_transferfunction[i].b);

        float v4 = (float)(m_transferfunction[i].a);

        if (!extinction_coef_type)
          v4 = MaterialOpacityToExtinction(v4);
        
        data[(i * 4) + 3] = v4;
      }
      ret->SetData((void*)data, GL_RGBA16F, GL_RGBA, GL_FLOAT);
      delete[] data;
      return ret;
    }
    return NULL;
  }

  void TransferFunction1D::Build ()
  {
    if (m_transferfunction)
      delete[] m_transferfunction;
    m_transferfunction = new glm::dvec4[max_density + 1];

    BuildLinear();

    printf ("lqc: Transfer Function 1D Built!\n");
    m_built = true;
  }

  glm::vec4 TransferFunction1D::Get (double value, double max_data_value)
  {
    if (!m_built)
      Build();

    if (max_data_value >= 0)
      value = value * (double(max_density) / max_data_value);

    if (value < 0.0f || value > (float)max_density)
      return glm::vec4 (0);

    // range: [0, max_density]
    if (fabs(value - (float)max_density) < 0.000001)
    {
      return glm::vec4(m_transferfunction[max_density]);
    }
    else
    {
      glm::dvec4 v1 = m_transferfunction[(int)value];
      glm::dvec4 v2 = m_transferfunction[((int)value) + 1];

      double t = value - (int)value;

      return glm::vec4((1.0 - t)*v1 + t*v2);
    }
  }

  float TransferFunction1D::GetOpc (double value, double max_input_value)
  {
    float val = Get(value, max_input_value).a;

    if (extinction_coef_type)
      return ExtinctionToMaterialOpacity(val);

    return val;
  }

  float TransferFunction1D::GetOpcN (double normalized_value)
  {
    float val = Get(normalized_value, 1.0).a;

    if (extinction_coef_type)
      return ExtinctionToMaterialOpacity(val);

    return val;
  }

  float TransferFunction1D::GetExt (double value, double max_input_value)
  {
    float val = Get(value, max_input_value).a;

    if (!extinction_coef_type)
      return MaterialOpacityToExtinction(val);

    return val;
  }

  float TransferFunction1D::GetExtN (double normalized_value)
  {
    float val = Get(normalized_value, 1.0).a;

    if (!extinction_coef_type)
      return MaterialOpacityToExtinction(val);

    return val;
  }

  void TransferFunction1D::PrintControlPoints ()
  {
    printf ("Print Transfer Function: Control Points\n");
    int rgb_pts = (int)m_cpt_rgb.size ();
    printf ("- Printing the RGB Control Points\n");
    printf ("  Format: \"Number: Red Green Blue, Isovalue\"\n");
    for (int i = 0; i < rgb_pts; i++)
    {
      printf ("  %d: %.2f %.2f %.2f, %d\n", i + 1, m_cpt_rgb[i].m_color.x, m_cpt_rgb[i].m_color.y, m_cpt_rgb[i].m_color.z, m_cpt_rgb[i].m_isoValue);
    }
    printf ("\n");

    int alpha_pts = (int)m_cpt_alpha.size ();
    printf ("- Printing the Alpha Control Points\n");
    printf ("  Format: \"Number: Alpha, Isovalue\"\n");
    for (int i = 0; i < alpha_pts; i++)
    {
      printf ("  %d: %.2f, %d\n", i + 1, m_cpt_alpha[i].m_color.w, m_cpt_alpha[i].m_isoValue);
    }
    printf ("\n");
  }

  void TransferFunction1D::PrintTransferFunction ()
  {
    printf ("Print Transfer Function: Control Points\n");
    printf ("  Format: \"IsoValue: Red Green Blue, Alpha\"\n");
    for (int i = 0; i < max_density + 1; i++)
    {
      printf ("%d: %.2f %.2f %.2f, %.2f\n", i, m_transferfunction[i].x
        , m_transferfunction[i].y, m_transferfunction[i].z, m_transferfunction[i].w);
    }
  }

  bool TransferFunction1D::Save ()//char* filename, TFFormatType format)
  {
    /*
    std::string filesaved;
    filesaved.append (RESOURCE_LIBLQC_PATH);
    filesaved.append ("TransferFunctions/");
    filesaved.append (filename);
    if (format == TFFormatType::LQC)
    {
    filesaved.append (".tf1d");
    std::ofstream myfile (filesaved.c_str ());
    if (myfile.is_open ())
    {
    myfile << 0 << "\n";
    myfile << (int)m_cpt_rgb.size () << "\n";
    for (int i = 0; i < (int)m_cpt_rgb.size (); i++)
    {
    myfile << m_cpt_rgb[i].m_color.x << " " <<
    m_cpt_rgb[i].m_color.y << " " <<
    m_cpt_rgb[i].m_color.z << " " <<
    m_cpt_rgb[i].m_isoValue << " " << "\n";
    }
    myfile << (int)m_cpt_alpha.size () << "\n";
    for (int i = 0; i < (int)m_cpt_alpha.size (); i++)
    {
    myfile << m_cpt_alpha[i].m_color.w << " " <<
    m_cpt_alpha[i].m_isoValue << " " << "\n";
    }
    myfile.close ();
    printf ("lqc: Transfer Function 1D Control Points Saved!\n");
    }
    else
    {
    printf ("lqc: Error on opening file at VRTransferFunction::Save().\n");
    }
    }
    */
    
    return true;
  }

  bool TransferFunction1D::Load ()//std::string filename, TFFormatType format)
  {
    /*
    std::string filesaved;
    filesaved.append (RESOURCE_LIBLQC_PATH);
    filesaved.append ("TransferFunctions/");
    filesaved.append (filename);
    if (format == TFFormatType::LQC)
    {
    filesaved.append (".tf1d");
    std::ifstream myfile (filesaved.c_str ());
    if (myfile.is_open ())
    {
    int init;
    myfile >> init;

    int cpt_rgb_size;
    myfile >> cpt_rgb_size;
    float r, g, b, a;
    int isovalue;
    for (int i = 0; i < cpt_rgb_size; i++)
    {
    myfile >> r >> g >> b >> isovalue;
    m_cpt_rgb.push_back (TransferControlPoint (r, g, b, isovalue));
    }

    int cpt_alpha_size;
    myfile >> cpt_alpha_size;
    for (int i = 0; i < cpt_alpha_size; i++)
    {
    myfile >> a >> isovalue;
    m_cpt_alpha.push_back (TransferControlPoint (a, isovalue));
    }
    myfile.close ();
    printf ("lqc: Transfer Function 1D Control Points Loaded!\n");
    return true;
    }
    else
    printf ("lqc: Error on opening file at VRTransferFunction::AddControlPointsReadFile().\n");
    }
    return false;
    */

    return true;
  }

  void TransferFunction1D::BuildLinear ()
  {
    for (int i = 0; i < (int)m_cpt_rgb.size() - 1; i++)
    {
      int i0 = m_cpt_rgb[i].m_isoValue;
      int i1 = m_cpt_rgb[i + 1].m_isoValue;

      glm::dvec3 diff = glm::dvec3(
        m_cpt_rgb[i + 1].m_color.r - m_cpt_rgb[i].m_color.r,
        m_cpt_rgb[i + 1].m_color.g - m_cpt_rgb[i].m_color.g,
        m_cpt_rgb[i + 1].m_color.b - m_cpt_rgb[i].m_color.b
      );

      for (int x = i0; x <= i1; x++)
      {
        double k = (double)(x - i0) / (double)(i1 - i0);

        m_transferfunction[x].r = m_cpt_rgb[i].m_color.r + diff.r * k;
        m_transferfunction[x].g = m_cpt_rgb[i].m_color.g + diff.g * k;
        m_transferfunction[x].b = m_cpt_rgb[i].m_color.b + diff.b * k;
      }
    }

    for (int i = 0; i < (int)m_cpt_alpha.size() - 1; i++)
    {
      int i0 = m_cpt_alpha[i].m_isoValue;
      int i1 = m_cpt_alpha[i + 1].m_isoValue;

      double diff = double(
        m_cpt_alpha[i + 1].m_color.a - m_cpt_alpha[i].m_color.a
        );

      for (int x = i0; x <= i1; x++)
      {
        double k = (double)(x - i0) / (double)(i1 - i0);

        m_transferfunction[x].a = m_cpt_alpha[i].m_color.a + diff * k;
      }
    }
  }
}