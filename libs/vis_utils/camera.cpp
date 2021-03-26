#include "camera.h"

#include <cstdio>
#include <iostream>

#include <math_utils/utils.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace vis
{
  CameraData::CameraData ()
  {
    cam_setup_name = "";
    c_type = 0;
    eye = glm::vec3(0.0);
    center = glm::vec3(0.0);
    up = glm::vec3(0.0);

    field_of_view_y = 45.0f;
    aspect_ratio = 1.0f;
    z_near = 1.0f;
    z_far = 5000.0f;

    z_axis = glm::vec3(0.0);
    y_axis = glm::vec3(0.0);
    x_axis = glm::vec3(0.0);
    c_eye = glm::vec3(0.0);
  }

  CameraData::CameraData (std::string setup_name, glm::vec3 ieye, glm::vec3 icenter, glm::vec3 iup)
  {
    cam_setup_name = setup_name;
    c_type = 0;
    eye = ieye;
    center = icenter;
    up = iup;
    
    field_of_view_y = 45.0f;
    aspect_ratio = 1.0f;
    z_near = 1.0f;
    z_far = 5000.0f;

    z_axis = glm::normalize(eye - center);
    x_axis = glm::normalize(glm::cross(up, z_axis));
    y_axis = glm::normalize(glm::cross(z_axis, x_axis));

    c_eye = eye;
  }

  CameraData::~CameraData ()
  {
  }

  CameraBehaviour::CameraBehaviour ()
  {
  }

  CameraBehaviour::~CameraBehaviour ()
  {
  }

  ArcBallCameraBehaviour::ArcBallCameraBehaviour ()
  {
  }

  ArcBallCameraBehaviour::~ArcBallCameraBehaviour ()
  {
  }

  Camera::Camera ()
  {
    f_curr_time_func = nullptr;
    f_curr_time_data = nullptr;

    radius = 50;
    speed = 0.001f;
    speed_radius = 1.0f;

    min_radius = 0;
    max_radius = 1000;

    perspective = true;

    speed_keyboard_movement = 1.0f;
    speed_keyboard_rotation = 1.0f;
    speed_mouse_rotation = 1.0f;

    m_changing_camera = false;

    c_behaviour = ArcBallCameraBehaviour();
  }

  Camera::Camera (float _radius, float _min_rad, float _max_rad)
  {
    f_curr_time_func = nullptr;
    f_curr_time_data = nullptr;

    radius = _radius;
    speed = 0.001f;
    speed_radius = 1.0f;
  
    min_radius = _min_rad;
    max_radius = _max_rad;
    
    perspective = true;

    speed_keyboard_movement = 1.0f;
    speed_keyboard_rotation = 1.0f;
    speed_mouse_rotation    = 1.0f;

    m_changing_camera = false;

    c_behaviour = ArcBallCameraBehaviour();
  }
  
  Camera::~Camera ()
  {
  }

  bool Camera::UpdatePositionAndRotations ()
  {


    return false;
  }

  bool Camera::Changing ()
  {
    return m_changing_camera;
  }

  bool Camera::KeyboardDown (unsigned char key, int x, int y)
  {
    return false;
  }

  bool Camera::KeyboardUp (unsigned char key, int x, int y)
  {
    return false;
  }

  int Camera::MouseButton (int bt, int st, int x, int y)
  {
    if (st == 0 && bt == 0) {
      arcball_on = true;
      changing_radius = false;

      last_my = cur_my = y;
      last_mx = cur_mx = x;
      m_changing_camera = false;
    }
    else if (st == 0 && (bt == 1 || bt == 2)) {
      speed_radius = bt == 1 ? 1.0f : 0.01f;
      changing_radius = true;
      arcball_on = false;

      last_my = cur_my = y;
      last_mx = cur_mx = x;
      m_changing_camera = false;
    }
    else if (st == 1 && bt == 0) {
      arcball_on = false;
    }
    else if (st == 1 && (bt == 1 || bt == 2)) {
      changing_radius = false;
      m_changing_camera = false;
    }

    return 0;
  }

  int Camera::MouseMotion (int x, int y)
  {
    if (arcball_on) {
      float xrot = -(y - last_my) * speed;
      float yrot = -(x - last_mx) * speed;

      glm::quat p = glm::quat(0, c_data.eye.x, c_data.eye.y, c_data.eye.z);

      glm::quat qy = glm::quat(cos(yrot), sin(yrot) * c_data.up);

      glm::vec3 loc_up = c_data.up;

      float max = 0.99f;
      float dt = glm::dot(glm::normalize(glm::vec3(c_data.center - c_data.eye)), loc_up);
      if ((dt > max&& xrot > 0.0f) || (dt < -max && xrot < 0.0f))
        xrot = 0.0f;

      glm::vec3 vr = glm::normalize(glm::cross(glm::normalize(glm::vec3(c_data.center - c_data.eye)), loc_up));
      glm::quat qx = glm::quat(cos(xrot), sin(xrot) * vr);

      glm::quat rq =
        glm::cross(glm::cross(glm::cross(glm::cross(qx, qy), p),
          glm::inverse(qy)), glm::inverse(qx));

      c_data.eye = glm::vec3(rq.x, rq.y, rq.z);


      last_mx = cur_mx;
      last_my = cur_my;
      cur_mx = x;
      cur_my = y;

      m_changing_camera = true;
      return 1;
    }
    //////////////////////////////////////////
    //////////////////////////////////////////
    if (changing_radius) {
      float ydiff = (y - last_my) * speed_radius;

      radius += ydiff;
      if (radius < min_radius)
        radius = min_radius;
      if (radius > max_radius)
        radius = max_radius;


      glm::vec3 c_e = glm::normalize(glm::vec3(c_data.eye - c_data.center));

      c_data.eye = c_e * radius;

      last_my = cur_my;
      cur_my = y;

      m_changing_camera = true;
      return 1;
    }

    return 0;
  }

  float Camera::GetSpeedKeyboardMovement ()
  {
    return speed_keyboard_movement;
  }

  void Camera::SetSpeedKeyboardMovement (float sskm)
  {
    speed_keyboard_movement = sskm;
  }

  float Camera::GetSpeedKeyboardRotation ()
  {
    return speed_keyboard_rotation;
  }

  void Camera::SetSpeedKeyboardRotation (float sskr)
  {
    speed_keyboard_rotation = sskr;
  }

  float Camera::GetSpeedMouseRotation ()
  {
    return speed_mouse_rotation;
  }

  void Camera::SetSpeedMouseRotation (float ssmr)
  {
    speed_mouse_rotation = ssmr;
  }
  
  void Camera::SetInitialConfig (glm::vec3 _center, glm::vec3 _up)
  {
    c_data.center = _center;
    c_data.eye = glm::vec3(_center.x, _center.y, _center.z + radius);
    c_data.up = _up;
  }
    
  void Camera::SetSpeedRadius (float spd)
  {
    speed_radius = spd;
  }
  
  glm::mat4 Camera::LookAt ()
  {
    return glm::lookAt(c_data.eye, c_data.center, c_data.up);
  }
 
  glm::mat4 Camera::Projection ()
  {
    if (perspective)
    {
      glm::mat4 perspection = glm::perspective(c_data.field_of_view_y,
                                               c_data.aspect_ratio,
                                               c_data.z_near, c_data.z_far);
      return perspection;
    }
    else
    {
      printf("TODO: Ortographic\n");
      return glm::mat4();
    }
  }
  
  glm::vec3 Camera::GetDir ()
  {
    return glm::normalize(c_data.center - c_data.eye);
  }
  
  void Camera::UpdateAspectRatio (float w, float h)
  {
    c_data.aspect_ratio = w / h;
  }
  
  float Camera::GetAspectRatio ()
  {
    return c_data.aspect_ratio;
  }
  
  float Camera::GetFovY ()
  {
    return c_data.field_of_view_y;
  }
  
  float Camera::GetTanFovY ()
  {
    return (float)tan(DEGREE_TO_RADIANS(GetFovY()) / 2.0);
  }
  
  void Camera::SetData (CameraData* data)
  {
    c_data.eye    = data->eye;
    c_data.center = data->center;
    c_data.up     = data->up;
  
    radius = glm::distance(c_data.eye, c_data.center);
  }
  
  void Camera::GetCameraVectors (glm::vec3* cforward, glm::vec3* cup, glm::vec3* cright)
  {
    (*cforward) = -GetDir();
    (*cright) = glm::normalize(glm::cross(c_data.up, (*cforward)));
    (*cup) = glm::normalize(glm::cross((*cforward), (*cright)));
  }

  glm::vec3 Camera::GetEye ()
  {
    return c_data.eye;
  }

  glm::vec3 Camera::GetZAxis ()
  {
    return c_data.z_axis;
  }
  
  glm::vec3 Camera::GetYAxis ()
  {
    return c_data.y_axis;
  }
  
  glm::vec3 Camera::GetXAxis ()
  {
    return c_data.x_axis;
  }
}

/*
namespace vis
{
  CameraData::CameraData ()
  {

  }

  CameraData::CameraData (std::string setup_name, glm::vec3 _eye_pos, glm::vec3 _center, glm::vec3 _up)
  {
    cam_setup_name = setup_name;
    eye_pos = _eye_pos;
    center = _center;
    up = _up;
  }

  CameraData::~CameraData ()
  {

  }

  CameraBehaviour::CameraBehaviour ()
  {
    m_ref_data = nullptr;
  }

  CameraBehaviour::~CameraBehaviour ()
  {
  }

  void CameraBehaviour::SetInitialState (glm::vec3 eyepos, glm::vec3 center, glm::vec3 up)
  {
    m_ref_data->view_eye = eyepos;
    m_ref_data->view_dir = glm::normalize(center - eyepos);

    m_ref_data->z_axis = -m_ref_data->view_dir;
    m_ref_data->x_axis = glm::normalize(glm::cross(m_ref_data->view_dir, up));
    m_ref_data->y_axis = glm::normalize(glm::cross(m_ref_data->z_axis, m_ref_data->x_axis));
  }

  void CameraBehaviour::SetCameraDataPointer (CameraData* rdata)
  {
    m_ref_data = rdata;
  }

  FlightCameraBehaviour::FlightCameraBehaviour ()
  {
    m_input_rotations = glm::vec3(0.0f);
  }

  FlightCameraBehaviour::~FlightCameraBehaviour ()
  {

  }

  bool FlightCameraBehaviour::ComputeKeyInputs (std::map<unsigned char, bool>& input_movements,
                                                float key_mov, float key_rot, double diff_time)
  {
    bool c_changed = false;
    if(input_movements['w'])
    {
      m_ref_data->view_eye = m_ref_data->view_eye + m_ref_data->view_dir * key_mov * float(diff_time);
      c_changed = true;
    }
    if(input_movements['s'])
    {
      m_ref_data->view_eye = m_ref_data->view_eye - m_ref_data->view_dir * key_mov * float(diff_time);
      c_changed = true;
    }

    if(input_movements['a'])
    {
      m_ref_data->view_eye = m_ref_data->view_eye - m_ref_data->x_axis * key_mov * float(diff_time);
      c_changed = true;
    }
    if(input_movements['d'])
    {
      m_ref_data->view_eye = m_ref_data->view_eye + m_ref_data->x_axis * key_mov * float(diff_time);
      c_changed = true;
    }

    if (input_movements['r'])
    {
      m_ref_data->view_eye = m_ref_data->view_eye + m_ref_data->y_axis * key_mov * float(diff_time);
      c_changed = true;
    }
    if (input_movements['f'])
    {
      m_ref_data->view_eye = m_ref_data->view_eye - m_ref_data->y_axis * key_mov * float(diff_time);
      c_changed = true;
    }

    // rotate keyboard
    if (input_movements['q'])
    {
      m_input_rotations.z = m_input_rotations.z - key_rot * float(diff_time);
      c_changed = true;
    }
    if (input_movements['e'])
    {
      m_input_rotations.z = m_input_rotations.z + key_rot * float(diff_time);
      c_changed = true;
    }

    if (input_movements['z'])
    {
      m_input_rotations.y = m_input_rotations.y - key_rot * float(diff_time);
      c_changed = true;
    }
    if (input_movements['x'])
    {
      m_input_rotations.y = m_input_rotations.y + key_rot * float(diff_time);
      c_changed = true;
    }

    if (input_movements['c'])
    {
      m_input_rotations.x = m_input_rotations.x - key_rot * float(diff_time);
      c_changed = true;
    }
    if (input_movements['v'])
    {
      m_input_rotations.x = m_input_rotations.x + key_rot * float(diff_time);
      c_changed = true;
    }
    return c_changed;
  }

  bool FlightCameraBehaviour::ComputeMouseInputs (std::map<unsigned int, bool>& input_mouse,
                                                  glm::vec2 mouse_diff, glm::vec2 mouse_last,
                                                  float speed_mouse_rotation, double diff_time)
  {
    bool c_changed = false;
    // rotate mouse
    if(input_mouse[0])
    {
      m_input_rotations = m_input_rotations + glm::vec3(-mouse_diff.y * speed_mouse_rotation,
                                                        -mouse_diff.x * speed_mouse_rotation, 0.0f);
      c_changed = true;
    }
    return c_changed;
  }

  void FlightCameraBehaviour::ApplyRotations ()
  {
    if(m_input_rotations.x != 0.0f)
    {
      m_ref_data->y_axis = glm::normalize(glm::rotate(m_ref_data->y_axis,
        m_input_rotations.x, m_ref_data->x_axis));
      m_ref_data->z_axis = glm::normalize(glm::rotate(m_ref_data->z_axis,
        m_input_rotations.x, m_ref_data->x_axis));

      m_ref_data->view_dir = -m_ref_data->z_axis;

      m_input_rotations.x = 0.0f;
    }

    if(m_input_rotations.y != 0.0f)
    {
      m_ref_data->x_axis = glm::normalize(glm::rotate(m_ref_data->x_axis,
        m_input_rotations.y, m_ref_data->y_axis));
      m_ref_data->z_axis = glm::normalize(glm::rotate(m_ref_data->z_axis,
        m_input_rotations.y, m_ref_data->y_axis));

      m_ref_data->view_dir = -m_ref_data->z_axis;

      m_input_rotations.y = 0.0f;
    }

    if(m_input_rotations.z != 0.0f)
    {
      m_ref_data->x_axis = glm::normalize(glm::rotate(m_ref_data->x_axis,
        m_input_rotations.z, m_ref_data->z_axis));
      m_ref_data->y_axis = glm::normalize(glm::rotate(m_ref_data->y_axis,
        m_input_rotations.z, m_ref_data->z_axis));

      m_input_rotations.z = 0.0f;
    }
  }

  ArcBallCameraBehaviour::ArcBallCameraBehaviour ()
  {
    m_global_rotations = glm::vec3(0.0f);
  }

  ArcBallCameraBehaviour::~ArcBallCameraBehaviour ()
  {

  }

  bool ArcBallCameraBehaviour::ComputeKeyInputs (std::map<unsigned char, bool>& input_movements,
                                                 float key_mov, float key_rot, double diff_time)
  {
    bool c_changed = false;
    if(input_movements['w'])
    {
      glm::vec3 auxv = m_ref_data->view_eye + m_ref_data->view_dir * key_mov * float(diff_time);
      if(glm::distance(auxv, m_ref_data->center) > cte_min_radius)
      {
        m_ref_data->view_eye = auxv;
        c_changed = true;
      }
    }
    if(input_movements['s'])
    {
      m_ref_data->view_eye = m_ref_data->view_eye - m_ref_data->view_dir * key_mov * float(diff_time);
      c_changed = true;
    }

    if(input_movements['a'])
    {
      m_global_rotations.y = m_global_rotations.y - key_rot * float(diff_time);
      c_changed = true;
    }
    if (input_movements['d'])
    {
      m_global_rotations.y = m_global_rotations.y + key_rot * float(diff_time);
      c_changed = true;
    }

    if(input_movements['r'])
    {
      m_global_rotations.x = m_global_rotations.x - key_rot * float(diff_time);
      c_changed = true;
    }
    if(input_movements['f'])
    {
      m_global_rotations.x = m_global_rotations.x + key_rot * float(diff_time);
      c_changed = true;
    }
    return c_changed;
  }

  bool ArcBallCameraBehaviour::ComputeMouseInputs (std::map<unsigned int, bool>& input_mouse,
                                                   glm::vec2 mouse_diff, glm::vec2 mouse_last,
                                                   float speed_mouse_rotation, double diff_time)
  {
    return false;
  }

  void ArcBallCameraBehaviour::ApplyRotations ()
  {
    if(m_global_rotations.x != 0.0f)
    {
      m_ref_data->z_axis = glm::normalize(
        glm::rotate(m_ref_data->z_axis, m_global_rotations.x, m_ref_data->x_axis)
      );
      m_ref_data->y_axis = glm::normalize(
        glm::cross(m_ref_data->z_axis, m_ref_data->x_axis)
      );
      m_ref_data->x_axis = glm::normalize(
        glm::cross(m_ref_data->y_axis, m_ref_data->z_axis)
      );

      float d_lenght = glm::distance(m_ref_data->view_eye, m_ref_data->center);

      m_ref_data->view_eye = m_ref_data->center + m_ref_data->z_axis * d_lenght;
      m_ref_data->view_dir = -m_ref_data->z_axis;

      m_global_rotations.x = 0.0f;
    }

    if(m_global_rotations.y != 0.0f)
    {
      m_ref_data->z_axis = glm::normalize(
        glm::rotate(m_ref_data->z_axis, m_global_rotations.y, m_ref_data->y_axis)
      );
      m_ref_data->x_axis = glm::normalize(
        glm::cross(m_ref_data->y_axis, m_ref_data->z_axis)
      );
      m_ref_data->y_axis = glm::normalize(
        glm::cross(m_ref_data->z_axis, m_ref_data->x_axis)
      );

      float d_lenght = glm::distance(m_ref_data->view_eye, m_ref_data->center);

      m_ref_data->view_eye = m_ref_data->center + m_ref_data->z_axis * d_lenght;
      m_ref_data->view_dir = -m_ref_data->z_axis;

      m_global_rotations.y = 0.0f;
    }
  }

  Camera::Camera ()
  {
    f_curr_time_func = nullptr;
    f_curr_time_data = nullptr;

    m_cam_data.aspect_ratio = 1.0f;

    m_cam_data.field_of_view_y = 45;
    m_cam_data.z_near = 0.1f;
    m_cam_data.z_far = 1000.0f;

    m_cam_data.x_axis = glm::vec3(1.0f, 0.0f, 0.0f);
    m_cam_data.y_axis = glm::vec3(0.0f, 1.0f, 0.0f);
    m_cam_data.z_axis = glm::vec3(0.0f, 0.0f, 1.0f);

    m_cam_data.view_dir = -m_cam_data.z_axis;
    m_cam_data.view_eye = glm::vec3(0.0f, 0.0f, 0.0f);

    speed_keyboard_movement = 1.0f;
    speed_keyboard_rotation = 0.05f;
    speed_mouse_rotation    = 0.05f;

    // bindings for translation
    input_movements['w'] = false;
    input_movements['s'] = false;
    input_movements['a'] = false;
    input_movements['d'] = false;
    input_movements['r'] = false;
    input_movements['f'] = false;
    // bindings for rotation (for 6 DOF)
    // . z rotation
    input_movements['q'] = false;
    input_movements['e'] = false;
    // . y rotation
    input_movements['z'] = false;
    input_movements['x'] = false;
    // . x rotation
    input_movements['c'] = false;
    input_movements['v'] = false;

    input_mouse[0] = false;

    mouse_diff = glm::vec2(0.0f, 0.0f);
    mouse_last = glm::vec2(0.0f, 0.0f);

    m_cam_behaviour = nullptr;
    current_behaviour = Camera::CAMERA_BEHAVIOUR::ARCBALL;
    SetCameraBehaviour(Camera::CAMERA_BEHAVIOUR::FLIGHT);
  }

  Camera::~Camera ()
  {
  }

  void Camera::SetCameraBehaviour (Camera::CAMERA_BEHAVIOUR cam_beha)
  {
    if(current_behaviour != cam_beha)
    {
      if(m_cam_behaviour) delete m_cam_behaviour;
      m_cam_behaviour = nullptr;

      if(cam_beha == CAMERA_BEHAVIOUR::ARCBALL)
      {
        m_cam_behaviour = new ArcBallCameraBehaviour();
      }
      else // if(cam_beha == CAMERA_BEHAVIOUR::FLIGHT) or any other
      {
        m_cam_behaviour = new FlightCameraBehaviour();
      }
      m_cam_behaviour->SetCameraDataPointer(&m_cam_data);
      current_behaviour = cam_beha;
    }
  }

  Camera::CAMERA_BEHAVIOUR Camera::GetCameraBehaviour ()
  {
    return current_behaviour;
  }

  void Camera::SetCameraData (vis::CameraData* cd)
  {
    SetInitialState(cd->eye_pos, cd->center, cd->up);
    SetCameraBehaviour((Camera::CAMERA_BEHAVIOUR)cd->cam_type);
  }

  void Camera::SetInitialState (glm::vec3 eyepos, glm::vec3 center, glm::vec3 up)
  {
    m_cam_behaviour->SetInitialState(eyepos, center, up);
  }

  void Camera::SetGetCurrentTimeFunction (GetCurrentTimeCallback f, void* d)
  {
    f_curr_time_func = f;
    f_curr_time_data = d;
    current_elapsed_time = GetCurrentTime();
  }

  glm::mat4 Camera::LookAt (bool use_glm_functions)
  {
    if (use_glm_functions)
      return glm::lookAt(m_cam_data.view_eye, m_cam_data.view_eye + m_cam_data.view_dir * ((m_cam_data.z_near + m_cam_data.z_far) * 0.5f), m_cam_data.y_axis);

    // Build the left matrix with the current camera basis
    // | xx xy xz  0 |
    // | yx yy yz  0 |
    // | zx zy zz  0 |
    // |  0  0  0  1 |
    glm::mat4 mat_orientation = glm::mat4(1.0f);
    mat_orientation[0][0] = m_cam_data.x_axis.x; mat_orientation[1][0] = m_cam_data.x_axis.y; mat_orientation[2][0] = m_cam_data.x_axis.z;
    mat_orientation[0][1] = m_cam_data.y_axis.x; mat_orientation[1][1] = m_cam_data.y_axis.y; mat_orientation[2][1] = m_cam_data.y_axis.z;
    mat_orientation[0][2] = m_cam_data.z_axis.x; mat_orientation[1][2] = m_cam_data.z_axis.y; mat_orientation[2][2] = m_cam_data.z_axis.z;

    // Build the left matrix with the current camera basis
    // | 1 0 0 -ex |
    // | 0 1 0 -ey |
    // | 0 0 1 -ez |
    // | 0 0 0   1 |
    glm::mat4 mat_translation = glm::mat4(1.0f);
    mat_translation[3][0] = -GetViewEye().x;
    mat_translation[3][1] = -GetViewEye().y;
    mat_translation[3][2] = -GetViewEye().z;

    // look_at = mat_orientation * mat_translate
    return mat_orientation * mat_translation;
    // . glm::mat4 eyepos = glm::inverse(mat_orientation) * look_at;
  }

  glm::mat4 Camera::Projection (Camera::PROJECTION_MATRICES pm, bool use_glm_functions)
  {
    if (pm == Camera::PROJECTION_MATRICES::PERSPECTIVE)
    {
      return glm::perspective(m_cam_data.field_of_view_y, m_cam_data.aspect_ratio,
                              m_cam_data.z_near, m_cam_data.z_far);
    }
    else // (pm == Camera::PROJECTION_MATRICES::ORTOGRAPHIC)
    {
      return glm::mat4();// ::ortho(-1.0f, 1.0, -1.0, 1.0);
    }
  }

  float Camera::GetFovX (float fov_y)
  {
    return 2.0f * glm::atan(glm::tan(DegreesToRadians(fov_y) * 0.5f) * m_cam_data.aspect_ratio);
  }

  float Camera::GetFovY (float fov_x)
  {
    return 2.0f * glm::atan(glm::tan(DegreesToRadians(fov_x) * 0.5f) * (1.0 / m_cam_data.aspect_ratio));
  }

  void Camera::UpdateAspectRatio (int screen_width, int screen_height)
  {
    m_cam_data.aspect_ratio = (float)screen_width / (float)screen_height;
  }

  void Camera::SetViewEye (glm::vec3 ve)
  {
    m_cam_data.view_eye = ve;
  }

  glm::vec3 Camera::GetViewEye ()
  {
    return m_cam_data.view_eye;
  }

  glm::vec3 Camera::GetViewDir ()
  {
    return m_cam_data.view_dir;
  }

  glm::vec3 Camera::GetXAxis ()
  {
    return m_cam_data.x_axis;
  }

  glm::vec3 Camera::GetYAxis ()
  {
    return m_cam_data.y_axis;
  }

  glm::vec3 Camera::GetZAxis ()
  {
    return m_cam_data.z_axis;
  }

  bool Camera::UpdatePositionAndOrientation ()
  {
    bool cam_changed = false;
    double old_curr_time = current_elapsed_time;
    current_elapsed_time = GetCurrentTime();
    double diff_time = current_elapsed_time - old_curr_time;

    // if the callback is null, consider 60 fps
    if(current_elapsed_time < 0.0) diff_time = 0.01666667;

    cam_changed = m_cam_behaviour->ComputeKeyInputs(input_movements, speed_keyboard_movement, speed_keyboard_rotation, diff_time)
                || m_cam_behaviour->ComputeMouseInputs(input_mouse, mouse_diff, mouse_last, speed_mouse_rotation, diff_time);
    m_cam_behaviour->ApplyRotations();

    // from "Modern OpenGL Camera" repository: https://github.com/hmazhar/moderngl_camera
    //// compute quaternion for pitch based on the camera pitch angle
    //glm::quat pitch_quat = glm::angleAxis(rotations.x, x_axis);
    //// determine heading quaternion from the camera up vector and the heading angle
    //glm::quat heading_quat = glm::angleAxis(rotations.y, y_axis);
    //// add the two quaternions
    //glm::quat temp = glm::cross(pitch_quat, heading_quat);
    //temp = glm::normalize(temp);
    //// update the direction from the quaternion
    //view_dir = glm::rotate(temp, view_dir);
    //// add the camera delta

    // reset mouse diff
    mouse_diff = glm::vec2(0.0f);

    // returns if camera was modified
    return cam_changed;
  }

  bool Camera::KeyboardDown (unsigned char key, int x, int y)
  {
    if(input_movements.count(key) > 0)
    {
      input_movements[key] = true;
      current_elapsed_time = GetCurrentTime();
      return true;
    }
    return false;
  }

  bool Camera::KeyboardUp (unsigned char key, int x, int y)
  {
    if(input_movements.count(key) > 0)
    {
      input_movements[key] = false;
      current_elapsed_time = GetCurrentTime();
      return true;
    }
    return false;
  }

  void Camera::MouseButton (int bt, int st, int x, int y)
  {
    if(input_mouse.count(bt))
    {
      input_mouse[bt] = (st == 0);
      mouse_last = glm::vec2(x, y);
      mouse_diff = glm::vec2(0, 0);
      current_elapsed_time = GetCurrentTime();
    }
  }

  int Camera::MouseMotion (int x, int y)
  {
    if(input_mouse[0])
    {
      mouse_diff = glm::vec2(x, y) - mouse_last;
      mouse_last = glm::vec2(x, y);
      current_elapsed_time = GetCurrentTime();
      return 1;
    }
    return 0;
  }

  float Camera::GetSpeedKeyboardMovement ()
  {
    return speed_keyboard_movement;
  }

  void Camera::SetSpeedKeyboardMovement (float sskm)
  {
    speed_keyboard_movement = sskm;
  }

  float Camera::GetSpeedKeyboardRotation ()
  {
    return speed_keyboard_rotation;
  }

  void Camera::SetSpeedKeyboardRotation (float sskr)
  {
    speed_keyboard_rotation = sskr;
  }

  float Camera::GetSpeedMouseRotation ()
  {
    return speed_mouse_rotation;
  }

  void Camera::SetSpeedMouseRotation (float ssmr)
  {
    speed_mouse_rotation = ssmr;
  }
}
*/