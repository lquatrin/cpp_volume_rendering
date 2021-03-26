#include "transferfunction.h"

#include <fstream>
#include <cstdlib>

namespace vis
{
  TransferControlPoint::TransferControlPoint (double r, double g, double b, int isovalue)
  {
    m_color.x = r;
    m_color.y = g;
    m_color.z = b;
    m_color.w = 1.0f;
    m_isoValue = isovalue;
  }

  TransferControlPoint::TransferControlPoint (double alpha, int isovalue)
  {
    m_color.x = 0.0f;
    m_color.y = 0.0f;
    m_color.z = 0.0f;
    m_color.w = alpha;
    m_isoValue = isovalue;
  }
}