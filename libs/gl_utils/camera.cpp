//http://www.belanecbn.sk/3dtutorials/index.php?id=36
#include "camera.h"

#include <math_utils/geometry.h>

#include <GL/glew.h>

namespace gl
{
  /*GLCamera::GLCamera ()
    //: Camera (0,0)
  {
    View = NULL;

    m_at  = lqc::Vector3f(0.0f, 0.0f, 0.0f);
    m_eye = lqc::Vector3f(0.0f, 0.0f, 5.0f);
    m_up  = lqc::Vector3f (0.0f, 1.0f, 0.0f);

    m_z_e = lqc::Vector3f::Normalize (m_at - m_eye);
    m_y_e = lqc::Vector3f::Normalize (m_up);
    m_x_e = lqc::Vector3f (0.0f, 0.0f, 1.0f); // Vector3f::Normalize (Cross (m_y_e, m_x_e));

    m_mov_speed = 1.0f;
  }

  GLCamera::GLCamera (int WindowWidth, int WindowHeight)
    : Camera ((float)WindowWidth, (float)WindowHeight)
  {
    m_eye = lqc::Vector3f(0.0f, 0.0f, 0.0f);
    m_at  = lqc::Vector3f (0.0f, 0.0f, 1.0f);
    m_up  = lqc::Vector3f(0.0f, 1.0f, 0.0f);

    Init (NULL);
  }

  GLCamera::GLCamera (int WindowWidth, int WindowHeight, const lqc::Vector3f& Pos, const lqc::Vector3f& Target, const lqc::Vector3f& Up)
    : Camera (WindowWidth, WindowHeight)
  {
    m_eye = Pos;
    m_at = Target;
    m_up = Up;
    m_up = lqc::Vector3f::Normalize (m_up);

    Init (NULL);
  }

  void GLCamera::Init (void* data)
  {
  }

  GLCamera::~GLCamera()
  {
  }

  void GLCamera::move (gl::GLCamera::CAM_MOVE direction)
  {
    switch(direction)
    {
    case CAM_MOVE::FORWARD:
      m_eye += m_mov_speed * m_z_e;
      break;
    case CAM_MOVE::BACK:
      m_eye -= m_mov_speed * m_z_e;
      break;
    case CAM_MOVE::LEFT:
      m_eye += m_mov_speed * m_x_e;
      break;
    case CAM_MOVE::RIGHT:
      m_eye -= m_mov_speed * m_x_e;
      break;
    case CAM_MOVE::UP:
      m_eye += m_mov_speed * m_y_e;
      break;
    case CAM_MOVE::DOWN:
      m_eye -= m_mov_speed * m_y_e;
      break;
    default:
      break;
    }
  }

  void GLCamera::rotate (lqc::Vector3f axis, float angle)
  {

  }

  void GLCamera::CalculateViewMatrix()
  {
    if(View)
    {
      *View = ViewMatrix (m_z_e, m_y_e, m_x_e, m_eye);
    }
  }

  void GLCamera::LookAt (lqc::Vector3f Reference, lqc::Vector3f Position, bool RotateAroundReference)
  {
    m_at = Reference;
    m_eye = Position;

    m_x_e = lqc::Vector3f::Normalize (Position - Reference);
    m_z_e = lqc::Vector3f::Normalize (lqc::Cross (m_y_e, m_x_e));
    m_y_e = lqc::Cross (m_x_e, m_z_e);

    if(!RotateAroundReference)
    {
      m_at = m_eye;
      m_eye += m_x_e * 0.05f;
    }

    CalculateViewMatrix();
  }

  void GLCamera::SetViewMatrixPointer(float *View)
  {
    //View = (glm::mat4x4*)View;

    CalculateViewMatrix();
  }

  //math
  lqc::Matrix4f GLCamera::ViewMatrix (const lqc::Vector3f &x, const lqc::Vector3f &y, const lqc::Vector3f &z, const lqc::Vector3f &position)
  {

    lqc::Matrix4f V =
    {
      {
        x.x, y.x, z.x, 0.0f,
        x.y, y.y, z.y, 0.0f,
        x.z, y.z, z.z, 0.0f,
        -lqc::Dot (x, position), -lqc::Dot (y, position),
        -lqc::Dot (z, position), 1.0f
      }
    };

    //Matrix4f V = Matrix4f (
    //glm::vec4 (x.x, y.x, z.x, 0.0f),
    //glm::vec4(x.y, y.y, z.y, 0.0f),
    //glm::vec4(x.z, y.z, z.z, 0.0f),
    //glm::vec4(- glm::dot(x, position),
    //- glm::dot(y, position),
    //- glm::dot(z, position),
    //1.0f)
    //);
    return V;
  }

  lqc::Vector4f GLCamera::getEye()
  {
    lqc::Vector4f ret;
    ret.x = m_eye.x;  ret.y = m_eye.y;
    ret.z = m_eye.z;  ret.w = 1.0f;

    return ret;
  }
   
  lqc::Matrix4f GLCamera::getViewMatrix ()
  {
    lqc::Matrix4f vMatrix = *View;
    return vMatrix;
  }

  lqc::Matrix4f GLCamera::LookAt ()
  {
    lqc::Matrix4f vMatrix = lqc::IDENTITY_MATRIX;
    //glm::mat4x4 V = glm::lookAt(m_eye, m_center, m_up);

    return vMatrix;
  }*/

}