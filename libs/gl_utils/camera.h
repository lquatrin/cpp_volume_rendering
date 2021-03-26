#ifndef GL_UTILS_CAMERA_H
#define GL_UTILS_CAMERA_H

#include <cstdlib>
#include <glm/glm.hpp>

namespace gl
{
  /*class GLCamera //: public lqc::Camera
  {
  public:
    virtual const char* GetNameClass () { return "GLCamera"; }

    virtual void Init (void* data = NULL);

  public:
    enum CAM_MOVE{
      FORWARD,
      BACK   ,
      LEFT   ,
      RIGHT  ,
      UP     ,
      DOWN   ,
    };

    GLCamera ();
    GLCamera (int WindowWidth, int WindowHeight);
    GLCamera (int WindowWidth, int WindowHeight, const glm::vec3& Pos, const glm::vec3& Target, const glm::vec3& Up);
    ~GLCamera ();

    void CalculateViewMatrix ();
    void LookAt (glm::vec3 Reference, glm::vec3 Position, bool RotateAroundReference = false);
    void SetViewMatrixPointer (float *View);

    void move (CAM_MOVE direction);
    void rotate (glm::vec3 axis, float angle);
    
    glm::mat4 getViewMatrix ();
    //TODO
    glm::mat4 LookAt();

    glm::mat4 getEye();

    void Rotate (float angle, float axis_x, float axis_y, float axis_z);
    
    glm::vec3 m_up;

  private:
    glm::mat4 *View;
    glm::mat4 ViewMatrix (const glm::vec3 &x, const glm::vec3 &y, const glm::vec3 &z, const glm::vec3 &position);
  };*/
}

#endif


/*

#ifndef CAMERA_H
#define	CAMERA_H

#include "math_3d.h"


class Camera
{
public:

    Camera(int WindowWidth, int WindowHeight);

    Camera(int WindowWidth, int WindowHeight, const Vector3f& Pos, const Vector3f& Target, const Vector3f& Up);

    bool OnKeyboard(int Key);

    void OnMouse(int x, int y);

    void OnRender();

    const Vector3f& GetPos() const
    {
        return m_pos;
    }

    const Vector3f& GetTarget() const
    {
        return m_target;
    }

    const Vector3f& GetUp() const
    {
        return m_up;
    }

private:

    void Init();
    void Update();

    Vector3f m_pos;
    Vector3f m_target;
    Vector3f m_up;

    int m_windowWidth;
    int m_windowHeight;

    float m_AngleH;
    float m_AngleV;

    bool m_OnUpperEdge;
    bool m_OnLowerEdge;
    bool m_OnLeftEdge;
    bool m_OnRightEdge;

    Vector2i m_mousePos;
};
*/

/*

class CCamera
{
protected:
	mat4x4 *View;

public:
	vec3 X, Y, Z, Reference, Position;

	CCamera();
	~CCamera();

	void CalculateViewMatrix();
	void LookAt(vec3 Reference, vec3 Position, bool RotateAroundReference = false);
	void Move(vec3 Movement);
	vec3 OnKeys(BYTE Keys, float FrameTime);
	void OnMouseMove(int dx, int dy);
	void OnMouseWheel(float zDelta);
	void SetViewMatrixPointer(float *View);
};
*/