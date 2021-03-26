#include "volrendernull.h"

#include "defines.h"
#include "volrenderbase.h"

#include <vis_utils/camera.h>
#include <volvis_utils/datamanager.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

NullRenderer::NullRenderer ()
{
}

NullRenderer::~NullRenderer ()
{
}
  
const char* NullRenderer::GetName ()
{
  return "Null Renderer";
}

const char* NullRenderer::GetAbbreviationName ()
{
  return "nrd";
}

void NullRenderer::Clean ()
{
  SetBuilt(false);
}

bool NullRenderer::Init (int shader_width, int shader_height)
{
  SetBuilt(true);
  return true;
}

bool NullRenderer::Update (vis::Camera* camera)
{
  view_matrix       = camera->LookAt();
  projection_matrix = camera->Projection();

  return true;
}
    
void NullRenderer::Redraw ()
{
#ifdef USING_FREEGLUT
  glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_LINE_BIT);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glMultMatrixf(glm::value_ptr(view_matrix));

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glMultMatrixf(glm::value_ptr(projection_matrix));

  glLineWidth(2.0f);

  glBegin(GL_LINES);
  glColor3f(1.0f, 0.0f, 0.0f);
  glVertex3f(-100.0f, 0.0f, 0.0f);
  glVertex3f(1000.0f, 0.0f, 0.0f);

  glColor3f(0.0f, 1.0f, 0.0f);
  glVertex3f(0.0f, -100.0f, 0.0f);
  glVertex3f(0.0f, 1000.0f, 0.0f);

  glColor3f(0.0f, 0.0f, 1.0f);
  glVertex3f(0.0f, 0.0f, -100.0f);
  glVertex3f(0.0f, 0.0f, 1000.0f);
  glEnd();

  glm::dvec3 v_min = m_ext_data_manager->GetCurrentGridVolume()->GetGridBBoxMin();
  glm::dvec3 v_max = m_ext_data_manager->GetCurrentGridVolume()->GetGridBBoxMax();
  
  glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
  glBegin(GL_LINES);
  glVertex3f(v_min.x, v_max.y, v_max.z);
  glVertex3f(v_max.x, v_max.y, v_max.z);
  glVertex3f(v_min.x, v_min.y, v_max.z);
  glVertex3f(v_max.x, v_min.y, v_max.z);

  glVertex3f(v_min.x, v_max.y, v_min.z);
  glVertex3f(v_max.x, v_max.y, v_min.z);
  glVertex3f(v_min.x, v_min.y, v_min.z);
  glVertex3f(v_max.x, v_min.y, v_min.z);

  glVertex3f(v_max.x, v_max.y, v_min.z);
  glVertex3f(v_max.x, v_max.y, v_max.z);
  glVertex3f(v_min.x, v_max.y, v_min.z);
  glVertex3f(v_min.x, v_max.y, v_max.z);

  glVertex3f(v_max.x, v_min.y, v_min.z);
  glVertex3f(v_max.x, v_min.y, v_max.z);
  glVertex3f(v_min.x, v_min.y, v_min.z);
  glVertex3f(v_min.x, v_min.y, v_max.z);

  glVertex3f(v_max.x, v_min.y, v_max.z);
  glVertex3f(v_max.x, v_max.y, v_max.z);
  glVertex3f(v_max.x, v_min.y, v_min.z);
  glVertex3f(v_max.x, v_max.y, v_min.z);

  glVertex3f(v_min.x, v_min.y, v_max.z);
  glVertex3f(v_min.x, v_max.y, v_max.z);
  glVertex3f(v_min.x, v_min.y, v_min.z);
  glVertex3f(v_min.x, v_max.y, v_min.z);
  glEnd();

  glColor4f(0.5f, 0.5f, 0.5f, 0.2f);
  glBegin(GL_QUADS);
  glVertex3f(v_min.x, v_min.y, v_max.z);
  glVertex3f(v_max.x, v_min.y, v_max.z);
  glVertex3f(v_max.x, v_max.y, v_max.z);
  glVertex3f(v_min.x, v_max.y, v_max.z);

  glVertex3f(v_max.x, v_min.y, v_max.z);
  glVertex3f(v_max.x, v_min.y, v_min.z);
  glVertex3f(v_max.x, v_max.y, v_min.z);
  glVertex3f(v_max.x, v_max.y, v_max.z);

  glVertex3f(v_max.x, v_min.y, v_min.z);
  glVertex3f(v_min.x, v_min.y, v_min.z);
  glVertex3f(v_min.x, v_max.y, v_min.z);
  glVertex3f(v_max.x, v_max.y, v_min.z);

  glVertex3f(v_min.x, v_min.y, v_min.z);
  glVertex3f(v_min.x, v_min.y, v_max.z);
  glVertex3f(v_min.x, v_max.y, v_max.z);
  glVertex3f(v_min.x, v_max.y, v_min.z);

  glVertex3f(v_min.x, v_max.y, v_min.z);
  glVertex3f(v_min.x, v_max.y, v_max.z);
  glVertex3f(v_max.x, v_max.y, v_max.z);
  glVertex3f(v_max.x, v_max.y, v_min.z);

  glVertex3f(v_min.x, v_min.y, v_max.z);
  glVertex3f(v_min.x, v_min.y, v_min.z);
  glVertex3f(v_max.x, v_min.y, v_min.z);
  glVertex3f(v_max.x, v_min.y, v_max.z);
  glEnd();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glPopAttrib();
#endif
}
  
vis::GRID_VOLUME_DATA_TYPE NullRenderer::GetDataTypeSupport ()
{
  return vis::GRID_VOLUME_DATA_TYPE::NONE_DATA_TYPE;
}
  
void NullRenderer::SetImGuiComponents ()
{
}