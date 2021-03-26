#ifndef VIS_UTILS_CAMERA_H
#define VIS_UTILS_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <iostream>

#include <map>

namespace vis
{
  class CameraData
  {
  public:
    CameraData ();
    CameraData (std::string setup_name, glm::vec3 ieye, glm::vec3 icenter, glm::vec3 iup);
    ~CameraData ();

    std::string cam_setup_name;
    unsigned int c_type;
    glm::vec3 eye, center, up;
  
    float aspect_ratio;
    float field_of_view_y;
    float z_near, z_far;
    glm::vec3 x_axis, y_axis, z_axis, c_eye;

  protected:

  private:
  };

  class CameraBehaviour
  {
  public:
    CameraBehaviour ();
    ~CameraBehaviour ();
  
  //  virtual void SetInitialState (glm::vec3 eyepos, glm::vec3 center, glm::vec3 up);
  //
  //  virtual bool ComputeKeyInputs (std::map<unsigned char, bool>& input_movements,
  //                                 float key_mov, float key_rot, double diff_time) = 0;
  //
  //  virtual bool ComputeMouseInputs (std::map<unsigned int, bool>& input_mouse,
  //                                   glm::vec2 mouse_diff, glm::vec2 mouse_last,
  //                                   float speed_mouse_rotation, double diff_time) = 0;
  //
  //  virtual void ApplyRotations() = 0;
  //
  //  void SetCameraDataPointer(CameraData* rdata);
  
  protected:
    CameraData* m_ref_data;
  
  private:
  };

  class ArcBallCameraBehaviour : public CameraBehaviour
  {
  public:
    ArcBallCameraBehaviour ();
    ~ArcBallCameraBehaviour ();

    //virtual void SetInitialState (glm::vec3 eyepos, glm::vec3 center, glm::vec3 up);

    //virtual bool ComputeKeyInputs(std::map<unsigned char, bool>& input_movements,
    //  float key_mov, float key_rot, double diff_time);
    //virtual bool ComputeMouseInputs (std::map<unsigned int, bool>& input_mouse,
    //                                 glm::vec2 mouse_diff, glm::vec2 mouse_last,
    //                                 float speed_mouse_rotation, double diff_time);
    //virtual void ApplyRotations();

  protected:
    //glm::vec3 m_global_rotations;
    //const float cte_min_radius = 10.0f;
  private:
  };

  class Camera
  {
  public:
    typedef double (*GetCurrentTimeCallback) (void* data);
    GetCurrentTimeCallback f_curr_time_func;
    void* f_curr_time_data;

    enum CAMERA_BEHAVIOUR : unsigned int {
      FLIGHT = 0,
      ARCBALL = 1,
    };
  
  public:
    Camera ();
    Camera (float _radius, float _min_rad = 1.0f, float _max_rad = 25000.0f);
    ~Camera ();

    bool UpdatePositionAndRotations ();

    bool Changing ();

    bool KeyboardDown (unsigned char key, int x, int y);
    bool KeyboardUp (unsigned char key, int x, int y);
    int MouseButton (int bt, int st, int x, int y);
    int MouseMotion (int x, int y);

    float GetSpeedKeyboardMovement ();
    void SetSpeedKeyboardMovement (float sskm);

    float GetSpeedKeyboardRotation ();
    void SetSpeedKeyboardRotation (float sskr);

    float GetSpeedMouseRotation ();
    void SetSpeedMouseRotation (float ssmr);

    void GenerateCameraVectors (glm::vec3* zaxis, glm::vec3* yaxis, glm::vec3* xaxis)
    {
      (*zaxis) = glm::normalize(-GetDir());
      
      (*xaxis) = glm::normalize(glm::cross(GetUp(), (*zaxis)));

      (*yaxis) = glm::normalize(glm::cross((*zaxis), (*xaxis)));
    }

    void SetInitialConfig (glm::vec3 _center, glm::vec3 _up);
    
    void SetSpeedRadius (float spd);
  
    glm::mat4 LookAt ();
    glm::mat4 Projection ();
  
    glm::vec3 GetDir ();
    glm::vec3 GetUp ()
    {
      return c_data.up;
    }
  
    float GetRadius ()
    {
      return radius;
    }
  
    void UpdateAspectRatio (float w, float h);

    float GetAspectRatio ();
    float GetFovY ();
    float GetTanFovY ();
  
    void SetData (CameraData* data);
  
    void GetCameraVectors (glm::vec3* forward, glm::vec3* up, glm::vec3* right);
  
    glm::vec3 GetEye ();
    glm::vec3 GetZAxis ();
    glm::vec3 GetYAxis ();
    glm::vec3 GetXAxis ();
  
  protected:
    CameraBehaviour c_behaviour;
    CameraData c_data;

    bool m_changing_camera;
    
    float radius;
    float speed;
    float speed_radius;
  
    float min_radius;
    float max_radius;
  
    bool perspective;
  
  private:
    int last_mx, last_my;
    int cur_mx, cur_my;
    bool arcball_on;
    bool changing_radius;

    float speed_keyboard_movement;
    float speed_keyboard_rotation;

    std::map<unsigned char, bool> input_movements;
    float speed_mouse_rotation;

    std::map<unsigned int, bool> input_mouse;
    glm::vec2 mouse_diff, mouse_last;
    double current_elapsed_time;
  };
}

#endif


/*


  class CameraBehaviour
  {
  public:
    CameraBehaviour ();
    ~CameraBehaviour ();

    virtual void SetInitialState (glm::vec3 eyepos, glm::vec3 center, glm::vec3 up);

    virtual bool ComputeKeyInputs (std::map<unsigned char, bool>& input_movements,
                                   float key_mov, float key_rot, double diff_time) = 0;

    virtual bool ComputeMouseInputs (std::map<unsigned int, bool>& input_mouse,
                                     glm::vec2 mouse_diff, glm::vec2 mouse_last,
                                     float speed_mouse_rotation, double diff_time) = 0;

    virtual void ApplyRotations () = 0;

    void SetCameraDataPointer (CameraData* rdata);

  protected:
    CameraData* m_ref_data;

  private:
  };

  class FlightCameraBehaviour : public CameraBehaviour
  {
  public:
    FlightCameraBehaviour ();
    ~FlightCameraBehaviour ();

    //virtual void SetInitialState (glm::vec3 eyepos, glm::vec3 center, glm::vec3 up);

    virtual bool ComputeKeyInputs (std::map<unsigned char, bool>& input_movements,
                                   float key_mov, float key_rot, double diff_time);
    virtual bool ComputeMouseInputs (std::map<unsigned int, bool>& input_mouse,
                                     glm::vec2 mouse_diff, glm::vec2 mouse_last,
                                     float speed_mouse_rotation, double diff_time);
    virtual void ApplyRotations ();

  protected:
    glm::vec3 m_input_rotations;
  private:
  };

  class ArcBallCameraBehaviour : public CameraBehaviour
  {
  public:
    ArcBallCameraBehaviour ();
    ~ArcBallCameraBehaviour ();

    //virtual void SetInitialState (glm::vec3 eyepos, glm::vec3 center, glm::vec3 up);

    virtual bool ComputeKeyInputs (std::map<unsigned char, bool>& input_movements,
                                   float key_mov, float key_rot, double diff_time);
    virtual bool ComputeMouseInputs (std::map<unsigned int, bool>& input_mouse,
                                     glm::vec2 mouse_diff, glm::vec2 mouse_last,
                                     float speed_mouse_rotation, double diff_time);
    virtual void ApplyRotations ();

  protected:
    glm::vec3 m_global_rotations;
    const float cte_min_radius = 10.0f;
  private:
  };

  class Camera
  {
  public:
    typedef double (*GetCurrentTimeCallback) (void* data);
    GetCurrentTimeCallback f_curr_time_func;
    void* f_curr_time_data;

    enum CAMERA_BEHAVIOUR : unsigned int {
      FLIGHT        = 0,
      // https://www.talisman.org/~erlkonig/misc/shoemake92-arcball.pdf
      ARCBALL       = 1,
    };

    enum PROJECTION_MATRICES : unsigned int {
      PERSPECTIVE = 0,
      ORTOGRAPHIC = 1,
    };

    Camera ();
    ~Camera ();

    void SetCameraBehaviour (Camera::CAMERA_BEHAVIOUR cam_beha);
    Camera::CAMERA_BEHAVIOUR GetCameraBehaviour ();

    void SetCameraData (vis::CameraData* cd);

    void SetInitialState (glm::vec3 eyepos, glm::vec3 center, glm::vec3 up);

    void SetGetCurrentTimeFunction (GetCurrentTimeCallback f, void* d);

    glm::mat4 LookAt (bool use_glm_functions = true);
    glm::mat4 Projection (Camera::PROJECTION_MATRICES pm = Camera::PROJECTION_MATRICES::PERSPECTIVE, bool use_glm_functions = true);

    // https://stackoverflow.com/questions/5504635/computing-fovx-opengl
    float GetFovX (float fov_y);
    float GetFovY ()
    {
      return m_cam_data.field_of_view_y;
    }
    float GetFovY (float fov_x);

    float GetAspectRatio ()
    {
      return m_cam_data.aspect_ratio;
    }
    void UpdateAspectRatio (int screen_width, int screen_height);

    void SetViewEye (glm::vec3 ve);
    glm::vec3 GetViewEye ();
    glm::vec3 GetViewDir ();

    glm::vec3 GetXAxis ();
    glm::vec3 GetYAxis ();
    glm::vec3 GetZAxis ();

    bool UpdatePositionAndOrientation ();
    bool KeyboardDown (unsigned char key, int x, int y);
    bool KeyboardUp (unsigned char key, int x, int y);
    void MouseButton (int bt, int st, int x, int y);
    int MouseMotion (int x, int y);

    float GetSpeedKeyboardMovement ();
    void SetSpeedKeyboardMovement (float sskm);

    float GetSpeedKeyboardRotation ();
    void SetSpeedKeyboardRotation (float sskr);

    float GetSpeedMouseRotation ();
    void SetSpeedMouseRotation (float ssmr);

  protected:
    CAMERA_BEHAVIOUR current_behaviour;
    CameraBehaviour* m_cam_behaviour;

    double GetCurrentTime ()
    {
      if (f_curr_time_func)
        return f_curr_time_func(f_curr_time_data);
      return -1.0;
    }

    CameraData m_cam_data;

    float speed_keyboard_movement;
    float speed_keyboard_rotation;
    std::map<unsigned char, bool> input_movements;
    float speed_mouse_rotation;
    std::map<unsigned int, bool> input_mouse;
    glm::vec2 mouse_diff, mouse_last;
    double current_elapsed_time;

  private:

  };
}

#endif
*/