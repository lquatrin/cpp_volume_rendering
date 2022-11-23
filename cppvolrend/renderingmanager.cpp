#include "renderingmanager.h"

#include "defines.h"

#include <glm/gtc/type_ptr.hpp>

#include "volrenderbase.h"
#include <gl_utils/framebufferobject.h>

#include <volvis_utils/transferfunction1d.h>

#include <volvis_utils/utils.h>

#define USING_IM_EXT
#ifdef USING_IM_EXT
#include <im/im.h>
#include <im/im_image.h>
#endif

#include <math_utils/utils.h>
#include <vis_utils/colorutils.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <iostream>
#include <iomanip>
#include <chrono>
#include <filesystem>

#include <GL/wglew.h>

#define ALWAYS_OUTDATE_THE_CURRENT_VR_RENDERER

#define RENDERING_MANAGER_MIN_VIEWPORT_SIZE 32
#define RENDERING_MANAGER_MAX_VIEWPORT_SIZE 8192
#define RENDERING_MANAGER_TIME_PER_FPS_COUNT_MS 5000.0

#define WHITE_BACKGROUND

//#define GET_NVIDIA_MEMORY
#ifdef GET_NVIDIA_MEMORY
#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049
#endif

double GetCurrentRenderTime()
{
#ifdef USING_FREEGLUT
  return glutGet(GLUT_ELAPSED_TIME);
#elif USING_GLFW
  return glfwGetTime();
#endif
}

RenderingManager *RenderingManager::crr_instance = 0;

RenderingManager* RenderingManager::Instance ()
{
  if (!crr_instance)
    crr_instance = new RenderingManager();

  return crr_instance;
}

bool RenderingManager::Exists ()
{
  return (crr_instance != NULL);
}

void RenderingManager::DestroyInstance ()
{
  if (!RenderingManager::Exists())
    return;

  if (crr_instance)
  {
    delete crr_instance;
    crr_instance = NULL;
  }
}

// Init glew + camera + curr vol renderer
void RenderingManager::InitGL()
{
#ifdef USING_FREEGLUT
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_TEXTURE_3D);
#else
#ifdef USING_GLFW
  // Deprecated on Opengl 4x, does not work with glfw
  //glEnable(GL_TEXTURE_2D);
  //glEnable(GL_TEXTURE_3D);
#endif
#endif
  glEnable(GL_DEPTH_TEST);
  gl::ExitOnGLError("RenderingManager: Could not enable depth test...");

  glEnable(GL_CULL_FACE);
  gl::ExitOnGLError("RenderingManager: Could not enable cull face...");

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
  gl::ExitOnGLError("RenderingManager: Could not enable blend...");

#ifdef WHITE_BACKGROUND
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
#else
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
#endif

  int max;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
  //std::cout << max << std::endl;
}

void RenderingManager::AddVolumeRenderer (BaseVolumeRenderer* bvolrend)
{
  m_vtr_vr_methods.push_back(bvolrend);
  m_vtr_vr_ui_names.push_back(bvolrend->GetName());
}

// Init glew + camera + curr vol renderer
void RenderingManager::InitData ()
{
  // Read Datasets and Transfer Functions from Data Manager
  m_data_mgr.SetPathToData(MAKE_STR(CMAKE_PATH_TO_DATA_FOLDER));
  m_data_mgr.ReadData();
 
  // Read camera states and set first camera data
  std::string path_data_folder1(MAKE_STR(CMAKE_PATH_TO_DATA_FOLDER));
  m_camera_state_list.ReadCameraStates(path_data_folder1 + "#list_camera_states");
  m_std_cam_state_names.clear();
  for (int i = 0; i < m_camera_state_list.NumberOfCameraStates(); i++)
    m_std_cam_state_names.push_back(m_camera_state_list.GetCameraState(i)->cam_setup_name);
  m_current_camera_state_id = 0;
  curr_rdr_parameters.GetCamera()->SetData(m_camera_state_list.GetCameraState(m_current_camera_state_id));

  // Read light source lists
  std::string path_data_folder2(MAKE_STR(CMAKE_PATH_TO_DATA_FOLDER));
  m_light_source_list.ReadLightSourceLists(path_data_folder2 + "#list_light_sources");
  m_std_lsource_names.clear();
  for (int i = 0; i < m_light_source_list.NumberOfLists(); i++)
    m_std_lsource_names.push_back(m_light_source_list.GetList(i)->l_name);
  m_current_lightsource_data_id = glm::clamp(m_current_lightsource_data_id, 0, m_light_source_list.NumberOfLists() - 1);

  curr_rdr_parameters.EraseAllLightSources();
  for (int i = 0; i < m_light_source_list.GetList(m_current_lightsource_data_id)->m_lightsources.size(); i++)
    curr_rdr_parameters.CreateNewLightSource(m_light_source_list.GetList(i)->m_lightsources[i]);

  if (m_vtr_vr_methods.empty())
  {
    std::cout << "RenderingManager: No VR method added." << std::endl;
    exit(1);
  }

  // Set auxiliar data parameter classes for each rendering mode
  for (int i = 0; i < m_vtr_vr_methods.size(); i++)
    m_vtr_vr_methods[i]->SetExternalResources(&m_data_mgr, &curr_rdr_parameters);

  // Current rendering mode id (Null)
  m_current_vr_method_id = 0;

  // Update rendering mode
  SetCurrentVolumeRenderer();
  UpdateLightSourceCameraVectors();
 
  // Reshape
  Reshape(curr_rdr_parameters.GetScreenWidth(), curr_rdr_parameters.GetScreenHeight());
}

void RenderingManager::Display ()
{
  //We always redraw during evaluation
  if (m_eval_running)
  {
    curr_vol_renderer->SetOutdated();
  }
  else
  {
    //The frame rate display (console and ImGui) is only updated when not evaluating.
    //Evaluation computes the frame rate after a given fixed number of frames.
    UpdateFrameRate();
  }

  if (curr_rdr_parameters.GetCamera()->UpdatePositionAndRotations())
  {
    curr_vol_renderer->SetOutdated();
  }

  // Build ImgGui interface
  if (m_imgui_render_ui) SetImGuiInterface();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // Render Function
  if (curr_vol_renderer && curr_vol_renderer->IsBuilt())
  {
    curr_vol_renderer->PrepareRender(curr_rdr_parameters.GetCamera());

#ifdef MULTISAMPLE_AVAILABLE
    f_render[curr_vol_renderer->GetCurrentMultiScalingMode()](this);
#else
    curr_vol_renderer->Redraw();
#endif
  }

  // If we must capture a screenshot
  if (curr_rdr_parameters.TakeScreenshot())
    SaveScreenshot();

  // Render the current ImGui interface
  if (m_imgui_render_ui) DrawImGuiInterface();

#ifdef USING_FREEGLUT
  // Swap buffer
  f_swapbuffer(d_swapbuffer);
#else
#ifdef USING_GLFW
  f_swapbuffer(d_swapbuffer);
#endif
#endif
  
  // If camera is rotating
  if (animate_camera_rotation)
  {
    //if (updating_world_light_pos)
    //{
    //  curr_rdr_parameters.SetBlinnPhongLightingPosition(curr_rdr_parameters.GetCamera()->GetEye());
    //  UpdateLightSourceCameraVectors();
    //  curr_vol_renderer->SetOutdated();
    //}

#ifdef USING_FREEGLUT
    double endframetime = glutGet(GLUT_ELAPSED_TIME);
#else
#ifdef USING_GLFW
    double endframetime = glfwGetTime() * 1000.0;
#endif
#endif
    double lwindowms = endframetime - m_ts_last_time;

    vis::CameraData sdata;
    float radius = curr_rdr_parameters.GetCamera()->GetRadius();
    sdata.center = glm::vec3(0.0f);
    sdata.up = curr_rdr_parameters.GetCamera()->GetUp();

    glm::vec3 cdir = -curr_rdr_parameters.GetCamera()->GetDir();
    
    glm::vec3 cright = glm::normalize(glm::cross(curr_rdr_parameters.GetCamera()->GetDir(),
                                                 curr_rdr_parameters.GetCamera()->GetUp()));
    cdir = RodriguesRotation(cdir, 0.0008f * ((float)lwindowms), sdata.up);
    sdata.eye = glm::vec3(cdir.x, cdir.y, cdir.z) * radius;

    curr_rdr_parameters.GetCamera()->SetData(&sdata);
    curr_vol_renderer->SetOutdated();
  }

  if (m_eval_running)
  {
    //We shoot a number of frames for each evaluation sample
    m_eval_currframe++; //go to the next frame
    if (m_eval_currframe >= m_eval_numframes)
    {
      //Compute the rendering speed for that sample point
      const double currenttime = GetCurrentRenderTime();
      const double time_per_frame = (currenttime - m_eval_lasttime) / m_eval_numframes;
      const double frames_per_second = 1000.0 / time_per_frame;

      //Save the last rendered image
      std::string imagefilename = std::to_string(m_eval_currsample);
      size_t n_zero = 4;
      imagefilename = std::string(n_zero - std::min(n_zero, imagefilename.length()), '0') + imagefilename + ".png";
      SaveScreenshot(m_eval_imgdirectory + "/" + imagefilename);

      //Store evaluation results in csv file
      for(int i=0;i<m_eval_paramspace.GetNumDimensions();i++)
      {
        m_eval_csvfile << m_eval_paramspace.GetDimensionValue(i) << ",";
      }
      m_eval_csvfile << std::to_string(time_per_frame) << ","
                     << std::to_string(frames_per_second) << ","
                     << "\"" << imagefilename << "\"\n";


      //We go to the next sample point in the parameter space.
      if (m_eval_paramspace.IncrEvaluation())
      {
        //The ID of the new sample point
        m_eval_currsample++;
        //... and we shoot as many frames there as for the other samples.
        m_eval_currframe = 0;
        //Restart time taking
        m_eval_lasttime = GetCurrentRenderTime();
      }
      else
      {
        //We reached the end of the evaluation.
        m_eval_paramspace.EndEvaluation();
        m_eval_running = false;
        m_eval_csvfile.close();
        //Enable or disable vsync according to user prefs
        if (m_vsync)
        {
          wglSwapIntervalEXT(1);
        }
        else
        {
          wglSwapIntervalEXT(0);
        }
        //Restore UI
        m_imgui_render_ui = true;
      }
    }
  }

}

void RenderingManager::Reshape (int w, int h)
{
  glViewport(0, 0, w, h);

  curr_rdr_parameters.SetScreenSize(w, h);
  curr_rdr_parameters.GetCamera()->UpdateAspectRatio(float(w), float(h));

  if (curr_vol_renderer->IsBuilt())
  {
    curr_vol_renderer->Reshape(w,h);
    curr_vol_renderer->SetOutdated();
  }

  PostRedisplay();
}

void RenderingManager::Keyboard (unsigned char key, int x, int y)
{
  curr_rdr_parameters.GetCamera()->KeyboardDown(key, x, y);

  PostRedisplay();
}

void RenderingManager::KeyboardUp(unsigned char key, int x, int y)
{
  curr_rdr_parameters.GetCamera()->KeyboardUp(key, x, y);

  if (key == 'i')
    m_imgui_render_ui = !m_imgui_render_ui;
  
  PostRedisplay();
}

void RenderingManager::MouseButton (int bt, int st, int x, int y)
{
  curr_rdr_parameters.GetCamera()->MouseButton(bt, st, x, y);

  PostRedisplay();
}

void RenderingManager::MouseMotion (int x, int y)
{
  if (curr_rdr_parameters.GetCamera()->MouseMotion(x, y) == 1)
  {

    curr_vol_renderer->SetOutdated();
  }
  
  PostRedisplay();
}

void RenderingManager::CloseFunc ()
{
  gl::PipelineShader::Unbind();
  gl::ArrayObject::Unbind();

  for (int i = m_vtr_vr_methods.size() - 1; i >= 0; i--) delete m_vtr_vr_methods[i];
  m_vtr_vr_methods.clear();
}

void RenderingManager::IdleFunc ()
{
  if (m_idle_rendering)
  {
#ifdef ALWAYS_OUTDATE_THE_CURRENT_VR_RENDERER
    curr_vol_renderer->SetOutdated();
#endif
    PostRedisplay();
  }
}

void RenderingManager::PostRedisplay ()
{
#ifdef USING_FREEGLUT
  glutPostRedisplay();
#else
#ifdef USING_GLFW
  Display();
#endif
#endif
}

// Update the volume renderer with the current volume and transfer function
void RenderingManager::UpdateDataAndResetCurrentVRMode ()
{
  curr_vol_renderer->Init(curr_rdr_parameters.GetScreenWidth(), curr_rdr_parameters.GetScreenHeight());
}

void RenderingManager::SaveScreenshot (std::string filename)
{
  // Get pixel data without alpha
  GLubyte* rgb_data = GetFrontBufferPixelData(false);
  if (filename.empty())
  {
    filename = AddAbreviationName(curr_rdr_parameters.GetDefaultScreenshotName());
  }
  GenerateImgFile(filename, curr_rdr_parameters.GetScreenWidth(), curr_rdr_parameters.GetScreenHeight(), rgb_data, false);
  delete[] rgb_data;
}

void RenderingManager::UpdateLightSourceCameraVectors ()
{
  glm::vec3 cforward, cup, cright;
  curr_rdr_parameters.GetCamera()->GetCameraVectors(&cforward, &cup, &cright);
  curr_rdr_parameters.SetBlinnPhongLightSourceCameraVectors(cforward, cup, cright);
}

void RenderingManager::ResetGLStateConfig ()
{
#ifdef USING_FREEGLUT
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
#endif
}

std::string RenderingManager::AddAbreviationName (std::string filename, std::string extension)
{
  std::string str = filename;
  std::size_t pos = str.find(".");
  std::string out_str = str.substr(0, pos);

  out_str = out_str + "_" + curr_vol_renderer->GetAbbreviationName() + extension;
  
  return out_str;
}

bool RenderingManager::GenerateImgFile (std::string out_str, int w, int h, unsigned char *gl_data, bool alpha, std::string image_type)
{
  int error;

#ifdef USING_IM_EXT
  //Deal with absolute and relative paths
  if (std::filesystem::path(out_str).is_relative())
  {
    std::string path_to_data = CPPVOLREND_DATA_DIR;
    out_str = path_to_data + out_str;
  }

  //Create the file and save it
  imFile* ifile = imFileNew(out_str.c_str(), image_type.c_str(), &error);
  int user_color_mode = alpha ? IM_RGB | IM_ALPHA | IM_PACKED : IM_RGB | IM_PACKED;
  error = imFileWriteImageInfo(ifile, w, h, user_color_mode, IM_BYTE);
  error = imFileWriteImageData(ifile, gl_data);
  imFileClose(ifile);
#else
  error = -1;
#endif

  return error == 0;
}

GLubyte* RenderingManager::GetFrontBufferPixelData (bool alpha)
{
  GLubyte *gl_img_data = new GLubyte[(alpha ? 4 : 3) * curr_rdr_parameters.GetScreenWidth() * curr_rdr_parameters.GetScreenHeight()];
  
  glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
  glPushAttrib(GL_PIXEL_MODE_BIT);
  glReadBuffer(GL_BACK);
  glFlush();
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  
  glReadPixels(0, 0, curr_rdr_parameters.GetScreenWidth(), curr_rdr_parameters.GetScreenHeight(), (alpha ? GL_RGBA : GL_RGB), GL_UNSIGNED_BYTE, gl_img_data);
  
  glPopAttrib();
  glPopClientAttrib();

  return gl_img_data;
}

// Go to previous renderer
bool RenderingManager::PreviousRenderer ()
{
  if (m_current_vr_method_id > 0)
  {
    ResetGLStateConfig();
    m_current_vr_method_id -= 1;
    SetCurrentVolumeRenderer();
    return true;
  }
  return false;
}

// Go to next renderer
bool RenderingManager::NextRenderer ()
{
  if (m_current_vr_method_id + 1 < m_vtr_vr_methods.size())
  {
    ResetGLStateConfig();
    m_current_vr_method_id += 1;
    SetCurrentVolumeRenderer();
    return true;
  }
  return false;
}

// Set current volume renderer
void RenderingManager::SetCurrentVolumeRenderer ()
{
  if (curr_vol_renderer) curr_vol_renderer->Clean();

  curr_vol_renderer = m_vtr_vr_methods[m_current_vr_method_id];
  if (curr_vol_renderer->GetDataTypeSupport() == m_data_mgr.GetInputVolumeDataType())
  {
    UpdateDataAndResetCurrentVRMode();
  }
  else
  {
    // Null Renderer
    m_current_vr_method_id = 0;
    curr_vol_renderer = m_vtr_vr_methods[m_current_vr_method_id];
    UpdateDataAndResetCurrentVRMode();
  }

  //Let the volume renderer define some parameters for evaluation
  curr_vol_renderer->FillParameterSpace(m_eval_paramspace);
}

void RenderingManager::SetImGuiInterface ()
{
#ifdef USING_IMGUI
  // Start the Dear ImGui frame
#ifdef USING_FREEGLUT
  ImGui_ImplOpenGL2_NewFrame();
  ImGui_ImplGLUT_NewFrame();
#else
#ifdef USING_GLFW
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
#endif
#endif

  ImGui::BeginMainMenuBar();
  if (ImGui::BeginMenu("File###FileMenu"))
  {
    if (ImGui::MenuItem("Exit###FileMenuExit"))
    {
      CloseFunc();
      exit(EXIT_SUCCESS);
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("View###ViewMenu"))
  {
    if (ImGui::MenuItem("Rendering Manager###ViewMenuRenderingManager"))
    {
      m_imgui_render_manager = true;
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Data Manager###ViewMenuDataManager"))
    {
      m_imgui_data_window = true;
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Render Modes###ViewMenuRendererManager"))
    {
      m_imgui_renderer_window = true;
    }
    ImGui::EndMenu();
  }
  ImGui::EndMainMenuBar();

  // https://eliasdaler.github.io/using-imgui-with-sfml-pt2/#using-imgui-with-stl
  static auto vector_getter = [](void* vec, int idx, const char** out_text)
  {
    auto& vector = *static_cast<std::vector<std::string>*>(vec);
    if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
    *out_text = vector.at(idx).c_str();
    return true;
  };

  if (m_imgui_render_manager)
  {
    ImGui::Begin("Rendering Manager", &m_imgui_render_manager);
    ImGui::BulletText("OpenGL Version %s", glGetString(GL_VERSION));
    if (ImGui::Checkbox("VSync", &m_vsync))
    {
      if (m_vsync)
      {
        wglSwapIntervalEXT(1);
      }
      else
      {
        wglSwapIntervalEXT(0);
      }
    };
    if (ImGui::Checkbox("Idle Redraw", &m_idle_rendering));
    ImGui::SameLine();
    ImGui::Text("- %.3f ms/frame (%.1f FPS)", m_ts_window_ms, m_ts_window_fps);
  
    if (ImGui::Checkbox("Rotate Camera", &animate_camera_rotation))
    {
      if (animate_camera_rotation) m_idle_rendering = true;
    }

    if (ImGui::Button("Save Screenshot"))
    {
      curr_rdr_parameters.SetDefaultScreenshotName("output.png");
      curr_rdr_parameters.RequestSaveScreenshot();
    }

    ImGui::Separator();
    if (ImGui::Button("Reference Image###BTNstorerefimage"))
    {
      s_ref_image.clear();
      glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

      int o_w = curr_rdr_parameters.GetScreenWidth(),
        o_h = curr_rdr_parameters.GetScreenHeight();
      s_ref_image.resize(o_w * o_h);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, curr_vol_renderer->GetScreenTextureID());
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, &s_ref_image[0]);
      glBindTexture(GL_TEXTURE_2D, 0);

      glPopAttrib();
    }
    ImGui::SameLine();
    if (ImGui::Button("Generate Diff###BTNgeneratediffimage"))
    {
      glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
      printf("Generate diff image\n");
      int o_w = curr_rdr_parameters.GetScreenWidth(),
        o_h = curr_rdr_parameters.GetScreenHeight();

      vis::TransferFunction1D tf1d;
      tf1d.AddAlphaControlPoint(vis::TransferControlPoint(1.0, 0));
      tf1d.AddAlphaControlPoint(vis::TransferControlPoint(1.0, 255));

      //tf1d.AddRGBControlPoint(vis::TransferControlPoint(1.0, 1.0, 1.0, 0.0));
      //tf1d.AddRGBControlPoint(vis::TransferControlPoint(1.0, 0.0, 0.0, (0.10 * 255.0)));
      //tf1d.AddRGBControlPoint(vis::TransferControlPoint(0.0, 1.0, 0.0, (0.55 * 255.0)));
      //tf1d.AddRGBControlPoint(vis::TransferControlPoint(0.0, 0.0, 1.0, (1.00 * 255.0)));

      tf1d.AddRGBControlPoint(vis::TransferControlPoint(1.0, 1.0, 1.0, 0.0));
      tf1d.AddRGBControlPoint(vis::TransferControlPoint(1.0, 0.0, 0.0, (0.40 * 255.0)));
      tf1d.AddRGBControlPoint(vis::TransferControlPoint(1.0, 0.0, 0.0, (1.00 * 255.0)));
      tf1d.Build();

      //if (s_tex_diff_ref_screen)
      {
        double max_c_alpha = 0.0;

        //GLfloat* refe_rgba_data = new GLfloat[o_w * o_h * 4];
        glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, s_tex_diff_ref_screen->GetTextureID());
        //glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, refe_rgba_data);
        //glBindTexture(GL_TEXTURE_2D, 0);

        GLfloat* curr_rgba_data = new GLfloat[o_w * o_h * 4];
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, curr_vol_renderer->GetScreenTextureID());
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, curr_rgba_data);
        glBindTexture(GL_TEXTURE_2D, 0);


        GLubyte* ext_rgb_data = new GLubyte[o_w * o_h * 3];
        for (int i = 0; i < o_w * o_h; i++)
        {
          double rgb_ref[3];
          rgb_ref[0] = s_ref_image[i].x * 255.0;
          rgb_ref[1] = s_ref_image[i].y * 255.0;
          rgb_ref[2] = s_ref_image[i].z * 255.0;

          double rgb_cur[3];
          rgb_cur[0] = curr_rgba_data[(i * 4) + 0] * 255.0;
          rgb_cur[1] = curr_rgba_data[(i * 4) + 1] * 255.0;
          rgb_cur[2] = curr_rgba_data[(i * 4) + 2] * 255.0;

          double diff_lab = Cie2000Comparison(rgb_ref, rgb_cur);

          //double c_alpha_r = glm::clamp((Cie2000Comparison(rgb_ref, rgb_cur) / 100.0) / s_diff_max_c_alpha, 0.0, 1.0);
          double c_alpha_r = glm::clamp((diff_lab / 100.0), 0.0, 1.0);

          glm::vec4 clr = tf1d.Get(c_alpha_r * 255.0);
          ext_rgb_data[(i * 3) + 0] = clr.x * 255.0;
          ext_rgb_data[(i * 3) + 1] = clr.y * 255.0;
          ext_rgb_data[(i * 3) + 2] = clr.z * 255.0;
        }
        std::string fl = "";
        fl.append("Image");
        fl.append("_diff");
        fl.append(".png");
        GenerateImgFile(fl, o_w, o_h, ext_rgb_data, false, "PNG");

        //delete[] refe_rgba_data;
        delete[] ext_rgb_data;
        delete[] curr_rgba_data;
      }

      glPopAttrib();
    }
    ImGui::Separator();
    if (ImGui::Button("Export LAB Diff Transfer Function Image###BTNexportimagetfdiff"))
    {
      int img_w = 256, img_h = 40;
      GLubyte* tf_rgb_img = new GLubyte[img_w * img_h * 3];
      
      vis::TransferFunction1D tf1d;
      tf1d.AddAlphaControlPoint(vis::TransferControlPoint(1.0, 0));
      tf1d.AddAlphaControlPoint(vis::TransferControlPoint(1.0, 255));

      tf1d.AddRGBControlPoint(vis::TransferControlPoint(1.0, 1.0, 1.0, 0.0)); 
      tf1d.AddRGBControlPoint(vis::TransferControlPoint(1.0, 0.0, 0.0, (0.40 * 255.0)));
      tf1d.AddRGBControlPoint(vis::TransferControlPoint(1.0, 0.0, 0.0, (1.00 * 255.0)));
      tf1d.Build();

      for (int i = 0; i < img_w; i++)
      {
        glm::vec4 clr = tf1d.Get((double(i) / double(img_w)) * 255.0);
        for (int j = 0; j < img_h; j++)
        {
          int idx = (i + (j * img_w)) * 3;
          tf_rgb_img[idx + 0] = clr.r * 255.0;
          tf_rgb_img[idx + 1] = clr.g * 255.0;
          tf_rgb_img[idx + 2] = clr.b * 255.0;
        }
      }

      {
        std::string out_tf_str = "transfer_function_diff.png";
        GenerateImgFile(
          out_tf_str,
          img_w, img_h,
          tf_rgb_img,
          false
        );
      }

      delete[] tf_rgb_img;
    }
    

    if (ImGui::Button("Export Transfer Function Image###BTNexportimagetf"))
    {
      int img_w = 256, img_h = 40;
      GLubyte* tf_rgb_img = new GLubyte[img_w * img_h * 3];
    
      for (int i = 0; i < img_w; i++)
      {
        glm::vec4 clr = m_data_mgr.GetCurrentTransferFunction()->Get((double(i) / double(img_w)) * 255.0);
        for (int j = 0; j < img_h; j++)
        {
          int idx = (i + (j * img_w)) * 3;
          tf_rgb_img[idx + 0] = clr.r * 255.0;
          tf_rgb_img[idx + 1] = clr.g * 255.0;
          tf_rgb_img[idx + 2] = clr.b * 255.0;
        }
      }
      
      {
        std::string out_tf_str = "transfer_function.png";
        GenerateImgFile(
          out_tf_str, 
          img_w, img_h, 
          tf_rgb_img,
          false
        );
      }

      delete[] tf_rgb_img;
    }



    ImGui::PushID("Viewport Data");
    ImGui::Text("- Viewport:");
    int v_size[2] = { curr_rdr_parameters.GetScreenWidth(), curr_rdr_parameters.GetScreenHeight() };
    if (ImGui::DragInt2("###WINDOWVIEWPORT", v_size, 1, RENDERING_MANAGER_MIN_VIEWPORT_SIZE, RENDERING_MANAGER_MAX_VIEWPORT_SIZE))
    {
#ifdef USING_FREEGLUT
      glutReshapeWindow(v_size[0], v_size[1]);
#else
#ifdef USING_GLFW
#endif
#endif
      curr_vol_renderer->SetOutdated();
    }
    ImGui::PopID();

    if (ImGui::CollapsingHeader("Evaluation###EvaluationHeader"))
    {
      const int numsamples = m_eval_paramspace.GetNumSamplePoints();
      ImGui::Text("%d parameters with %d total samples", m_eval_paramspace.GetNumDimensions(), numsamples);

      ImGui::PushItemWidth(100);
      ImGui::InputInt("Eval Frames per Sample", &m_eval_numframes, 1, 10);
      ImGui::PopItemWidth();
      m_eval_numframes = std::max(std::min(m_eval_numframes, 500), 1);

      const double neededtime = ceil(m_ts_window_ms * numsamples * m_eval_numframes / 1000.);
      ImGui::Text("approx. %.0f seconds for evaluation at current FPS", neededtime);
      if (ImGui::Button("Start Evaluation"))
      {
        //Name of a directory containing all the evaluation files - naming is datetime-based
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "eval_%d-%m-%Y_%H-%M-%S");
        const std::string path_to_data = CPPVOLREND_DATA_DIR;
        m_eval_basedirectory = path_to_data + oss.str();
        // - and an image subsirectory
        m_eval_imgdirectory = m_eval_basedirectory + "/img";

        //Create the new directories
        std::filesystem::create_directory(m_eval_basedirectory);
        std::filesystem::create_directory(m_eval_imgdirectory);

        //Open a CSV file to record the results of the evaluation.
        if (m_eval_csvfile.is_open()) m_eval_csvfile.close(); //Should really not happen, but better be save.
        m_eval_csvfile.open(m_eval_basedirectory + "/eval.csv", std::ios_base::out);

        //Start the evaluation if everything is fine
        if (m_eval_csvfile.is_open())
        {
          //Reset parameter space to the beginning
          m_eval_paramspace.StartEvaluation();

          //Initialize the csv file
          for(int i=0;i<m_eval_paramspace.GetNumDimensions();i++)
          {
            m_eval_csvfile << m_eval_paramspace.GetDimensionName(i) << ",";
          }
          m_eval_csvfile << "TimePerFrame (ms),FramesPerSecond,ImageFile\n";

          //Set a bool to trigger evaluation action in Display().
          m_eval_running = true;
          m_eval_currframe = 0;
          m_eval_currsample = 0;
          m_eval_lasttime = GetCurrentRenderTime();
          curr_vol_renderer->SetOutdated();
          // Careful: Not rendering the ImGui may have unintended consequences,
          // namely if they Gui code changes parameters based on the parameters
          // that we are setting during the eval.
          // On the other hand, we want to measure the speed of the volume renderer and not of ImGui.
          m_imgui_render_ui = false;

          //Disable VSync for full speed
          wglSwapIntervalEXT(0);
        }
      }
    }

    //int u_cam_beha = m_camera.GetCameraBehaviour();
    if(ImGui::CollapsingHeader("Camera###CameraSettingsHeader"))
    {
    //  std::vector<std::string> vec_cam_states;
      ImGui::Text("- Load Camera State: ");
      if(ImGui::Combo("###ImArrayLoadCurrentCameraState", &m_current_camera_state_id, vector_getter,
        static_cast<void*>(&m_std_cam_state_names), m_std_cam_state_names.size()))
      {
        curr_rdr_parameters.GetCamera()->SetData(m_camera_state_list.GetCameraState(m_current_camera_state_id));
        curr_vol_renderer->SetOutdated();
      }
    //  ImGui::Text("- Behaviour: ");
    //  static const char* items_camera_behaviours[]{
    //    "Flight",
    //    "ArcBall",
    //  };
    //  if(ImGui::Combo("###CurrentCameraBehaviour", &u_cam_beha, 
    //     items_camera_behaviours, IM_ARRAYSIZE(items_camera_behaviours)))
    //  {
    //    m_camera.SetCameraBehaviour((vis::Camera::CAMERA_BEHAVIOUR)u_cam_beha);
    //  }
    }

    if (ImGui::CollapsingHeader("Light Sources###LightSourceManagerHeader"))
    {
      ImGui::Text("- Light Source List:");
      if (ImGui::Combo("###CurrentLightSourcePositionData", &m_current_lightsource_data_id, vector_getter,
        static_cast<void*>(&m_std_lsource_names), m_std_lsource_names.size()))
      {
        //updating_world_light_pos = false;

        curr_rdr_parameters.EraseAllLightSources();
        for (int i = 0; i < m_light_source_list.GetList(m_current_lightsource_data_id)->m_lightsources.size(); i++)
        {
          vis::LightSourceData lsd;
          lsd.position = m_light_source_list.GetList(m_current_lightsource_data_id)->m_lightsources[i].position;
          lsd.z_axis = m_light_source_list.GetList(m_current_lightsource_data_id)->m_lightsources[i].z_axis;
          lsd.y_axis = m_light_source_list.GetList(m_current_lightsource_data_id)->m_lightsources[i].y_axis;
          lsd.x_axis = m_light_source_list.GetList(m_current_lightsource_data_id)->m_lightsources[i].x_axis;
          curr_rdr_parameters.CreateNewLightSource(lsd);
        }

        curr_vol_renderer->SetOutdated();
      }

      if (ImGui::Button("New Light Source###RMANAGERADDLIGHTSOURCE"))
      {
        curr_rdr_parameters.CreateNewLightSource();
        curr_vol_renderer->SetOutdated();
      }
    
      int remove_light_source = -1;
      for (int i = 0; i < curr_rdr_parameters.GetNumberOfLightSources(); i++)
      {
        ImGui::Separator();
        ImGui::PushItemWidth(60.0);
        vis::LightSourceData* lsd = curr_rdr_parameters.GetLightSourceData(i);
    
        std::string lsourcepath = std::to_string(i + 1);
        ImGui::Text(lsourcepath.c_str());
        ImGui::SameLine();
        std::string lsp_bt_cam = "Copy From Camera###RMLightSourceFromCamerabt";
        lsp_bt_cam.append(lsourcepath);
        if (ImGui::Button(lsp_bt_cam.c_str()))
        {
          lsd->position = curr_rdr_parameters.GetCamera()->GetEye();
          lsd->z_axis   = -curr_rdr_parameters.GetCamera()->GetDir();
          lsd->y_axis   = curr_rdr_parameters.GetCamera()->GetYAxis();
          lsd->x_axis   = curr_rdr_parameters.GetCamera()->GetXAxis();
          curr_vol_renderer->SetOutdated();
        }
        ImGui::SameLine();
        std::string lsp_bt = "x###RMLightSourceParameterbt";
        lsp_bt.append(lsourcepath);
        if (ImGui::Button(lsp_bt.c_str()))
        {
          curr_vol_renderer->SetOutdated();
          remove_light_source = i;
        }
    
        {
          ImGui::BulletText("Position: ");
          ImGui::SameLine();
          std::string lsp_v1 = "###RMLightSourceParameterv1";
          lsp_v1.append(lsourcepath);
          if (ImGui::DragFloat(lsp_v1.c_str(), &lsd->position.x, 0.5f, -FLT_MAX, FLT_MAX, "%.1f"))
          {
            curr_vol_renderer->SetOutdated();
          }
    
          ImGui::SameLine();
          std::string lsp_v2 = "###RMLightSourceParameterv2";
          lsp_v2.append(lsourcepath);
          if (ImGui::DragFloat(lsp_v2.c_str(), &lsd->position.y, 0.5f, -FLT_MAX, FLT_MAX, "%.1f"))
          {
            curr_vol_renderer->SetOutdated();
          }
    
          ImGui::SameLine();
          std::string lsp_v3 = "###RMLightSourceParameterv3";
          lsp_v3.append(lsourcepath);
          if (ImGui::DragFloat(lsp_v3.c_str(), &lsd->position.z, 0.5f, -FLT_MAX, FLT_MAX, "%.1f"))
          {
            curr_vol_renderer->SetOutdated();
          }
        }

        ImGui::BulletText("Energy Density: ");
        ImGui::SameLine();
        std::string lsp_ed = "###RMLightSourceParameterEnergyDensity";
        lsp_ed.append(lsourcepath);
        if (ImGui::DragFloat(lsp_ed.c_str(), &lsd->energy_density, 0.1f, 0.0f, FLT_MAX, "%.1f"))
        {
          curr_vol_renderer->SetOutdated();
        }

        {
          ImGui::BulletText("Color: ");
          ImGui::SameLine();
          std::string lsp_clr_v1 = "###RMLightSourceParameterColorv1";
          lsp_clr_v1.append(lsourcepath);
          if (ImGui::DragFloat(lsp_clr_v1.c_str(), &lsd->color.x, 0.01f, 0.0f, 1.0f, "%.2f"))
          {
            curr_vol_renderer->SetOutdated();
          }

          ImGui::SameLine();
          std::string lsp_clr_v2 = "###RMLightSourceParameterColorv2";
          lsp_clr_v2.append(lsourcepath);
          if (ImGui::DragFloat(lsp_clr_v2.c_str(), &lsd->color.y, 0.01f, 0.0f, 1.0f, "%.2f"))
          {
            curr_vol_renderer->SetOutdated();
          }

          ImGui::SameLine();
          std::string lsp_clr_v3 = "###RMLightSourceParameterColorv3";
          lsp_clr_v3.append(lsourcepath);
          if (ImGui::DragFloat(lsp_clr_v3.c_str(), &lsd->color.z, 0.01f, 0.0f, 1.0f, "%.2f"))
          {
            curr_vol_renderer->SetOutdated();
          }
        }

        ImGui::PopItemWidth();

        static const char* items_rendermode[]{
          "Point Light",
          "Directional",
          "Spot Light",
        };
        int ilt = lsd->l_type;
        std::string lsp_ltype = "###RMLightSourceType";
        lsp_ltype.append(lsourcepath);
        if (ImGui::Combo(lsp_ltype.c_str(), &ilt, items_rendermode, IM_ARRAYSIZE(items_rendermode)))
        {
          lsd->l_type = (vis::LIGHT_SOURCE_TYPE)ilt;
        }
      }

      // remove the light source, if requested
      if (remove_light_source >= 0)
      {
        curr_rdr_parameters.EraseLightSource(remove_light_source);
      }
    }
    ImGui::End();
  }

  if (m_imgui_data_window)
  {
    ImGui::Begin("Data Manager###DataManagerWindow", &m_imgui_data_window);
    if (ImGui::CollapsingHeader("Input Volume###DataManagerInputVolume"))
    {
      // Show info for structured datasets...
      if (m_data_mgr.GetInputVolumeDataType() == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
      {
        if (m_data_mgr.GetNumberOfStructuredDatasets() > 0)
        {
          int volume_index = m_data_mgr.GetCurrentVolumeIndex();
          if (ImGui::Combo("###CurrentVolumeDataID", &volume_index, vector_getter,
            static_cast<void*>(m_data_mgr.GetUINameDatasetListPtr()), m_data_mgr.GetUINameDatasetListPtr()->size()))
          {
            if (volume_index != m_data_mgr.GetCurrentVolumeIndex())
            {
              m_data_mgr.SetCurrentInputVolume(volume_index);
              UpdateDataAndResetCurrentVRMode();
            }
          }
          ImGui::SameLine();
          if (ImGui::Button("<###PreviousVolume"))
          {
            m_data_mgr.PreviousVolume();
            UpdateDataAndResetCurrentVRMode();
          }
          ImGui::SameLine();
          if (ImGui::Button(">###NextVolume"))
          {
            m_data_mgr.NextVolume();
            UpdateDataAndResetCurrentVRMode();
          }
        }

        if (m_data_mgr.GetCurrentStructuredVolume() != nullptr)
        {
          ImGui::BulletText("Regular Grid");

          ImGui::BulletText("Resolution: %d %d %d", m_data_mgr.GetCurrentStructuredVolume()->GetWidth()
                                                  , m_data_mgr.GetCurrentStructuredVolume()->GetHeight()
                                                  , m_data_mgr.GetCurrentStructuredVolume()->GetDepth());
          ImGui::BulletText("Voxel Size: %.2f %.2f %.2f", m_data_mgr.GetCurrentStructuredVolume()->GetScaleX()
                                                        , m_data_mgr.GetCurrentStructuredVolume()->GetScaleY()
                                                        , m_data_mgr.GetCurrentStructuredVolume()->GetScaleZ());
        }
        
        if (ImGui::CollapsingHeader("Gradient Volume###DataManagerGradientVolume"))
        {
          int gradient_gen_index = m_data_mgr.GetCurrentGradientGenerationTypeID();
          std::vector<std::string> g_list = m_data_mgr.GetGradientGenerationTypeStrList();
          if (ImGui::Combo("###CurrentGradientGenerationDataID", &gradient_gen_index, vector_getter,
            static_cast<void*>(&g_list), g_list.size()))
          {
            if (gradient_gen_index != m_data_mgr.GetCurrentGradientGenerationTypeID())
            {
              m_data_mgr.SetCurrentGradient(gradient_gen_index);
              m_data_mgr.UpdateStructuredGradientTexture();
              UpdateDataAndResetCurrentVRMode();
            }
          }
        }
      }
    }
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Transfer Function###DataMdanagerTransferFunction"))
    {
      int transfer_function_index = m_data_mgr.GetCurrentTransferFunctionIndex();
      if (ImGui::Combo("###CurrentTransferFunction1DID", &transfer_function_index, vector_getter,
        static_cast<void*>(m_data_mgr.GetUINameTransferFunctionListPtr()), m_data_mgr.GetUINameTransferFunctionListPtr()->size()))
      {
        if (transfer_function_index != m_data_mgr.GetCurrentTransferFunctionIndex())
        {
          m_data_mgr.SetCurrentTransferFunction(transfer_function_index);
          UpdateDataAndResetCurrentVRMode();
        }
      }
      ImGui::SameLine();
      if (ImGui::Button("<###PreviousTF"))
      {
        m_data_mgr.PreviousTransferFunction();
        UpdateDataAndResetCurrentVRMode();
      }
      ImGui::SameLine();
      if (ImGui::Button(">###NextTF"))
      {
        m_data_mgr.NextTransferFunction();
        UpdateDataAndResetCurrentVRMode();
      }
    }
    ImGui::End();
  }

  if (m_imgui_renderer_window)
  {
    ImGui::Begin("Render Method###RendererWindow", &m_imgui_renderer_window);
    if (ImGui::Combo("###CurrentBaseVolumeRenderer", &m_current_vr_method_id, vector_getter,
      static_cast<void*>(&m_vtr_vr_ui_names), m_vtr_vr_ui_names.size()))
    {
      ResetGLStateConfig();
      SetCurrentVolumeRenderer();
    }

    if (ImGui::Button("Reload Shaders"))
    {
      curr_vol_renderer->ReloadShaders();
    }

    curr_vol_renderer->SetImGuiComponents();

    ImGui::End();
  }
   
  // Rendering
  ImGui::Render();
  ImGuiIO& io = ImGui::GetIO();
#endif
}

void RenderingManager::DrawImGuiInterface ()
{
#ifdef USING_IMGUI

#ifdef USING_FREEGLUT
  //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound, but prefer using the GL3+ code.
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_3D);
  ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
  glPopAttrib();
#else
#ifdef USING_GLFW
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif
#endif

#endif
}

void RenderingManager::UpdateFrameRate ()
{
  // Measure speed
  m_ts_current_time = GetCurrentRenderTime();
  m_ts_n_frames++;
  // After X seconds, compute frames per second...
  if ((m_ts_current_time - m_ts_last_time) > RENDERING_MANAGER_TIME_PER_FPS_COUNT_MS)
  {
    m_ts_window_fps = double(m_ts_n_frames) * 1000.0 / (m_ts_current_time - m_ts_last_time);
    m_ts_window_ms = 1000.0 / m_ts_window_fps;

    m_ts_last_time = m_ts_current_time;
    m_ts_n_frames = 0;
    printf("%.2lf frames per second\n", m_ts_window_fps);
  }
}

void RenderingManager::SingleSampleRender (void* data)
{
  RenderingManager* rm = (RenderingManager*)data;
  rm->curr_vol_renderer->Redraw();
}

void RenderingManager::MultiSampleRender (void* data)
{
  RenderingManager* rm = (RenderingManager*)data;
  rm->curr_vol_renderer->MultiSampleRedraw();
}

void RenderingManager::DownScalingRender (void* data)
{
  RenderingManager* rm = (RenderingManager*)data;
  rm->curr_vol_renderer->DownScalingRedraw();
}

void RenderingManager::UpScalingRender (void* data)
{
  RenderingManager* rm = (RenderingManager*)data;
  rm->curr_vol_renderer->UpScalingRedraw();
}

RenderingManager::RenderingManager ()
  : m_current_vr_method_id(0)
  , curr_vol_renderer(NULL)
{
  f_swapbuffer = nullptr;
  d_swapbuffer = nullptr;

  f_render[0] = RenderingManager::SingleSampleRender;
  f_render[1] = RenderingManager::MultiSampleRender;
  f_render[2] = RenderingManager::DownScalingRender;
  f_render[3] = RenderingManager::UpScalingRender;

  m_current_camera_state_id = 0;
  m_current_lightsource_data_id = 0;
  m_current_lightsource_data_id = 0;

  m_vtr_vr_ui_names.clear();
  m_current_vr_method_id = -1;
  curr_vol_renderer = nullptr;
  m_vtr_vr_methods.clear();

  animate_camera_rotation = false;

  m_eval_running = false;
  m_eval_numframes = 100;
  m_eval_currframe = 0;

  m_imgui_render_ui = true;

  m_imgui_render_manager  = true;
  m_idle_rendering        = true;
  m_vsync                 = true;
  m_ts_current_time = 0.0;
#ifdef USING_FREEGLUT
  m_ts_last_time = glutGet(GLUT_ELAPSED_TIME);
#else
#ifdef USING_GLFW
  m_ts_last_time = glfwGetTime();
#endif
#endif
  m_ts_n_frames = 0;
  m_ts_window_fps = 0.0;
  m_ts_window_ms = -1;

  m_imgui_data_window     = true;

  m_imgui_renderer_window = true;
}

RenderingManager::~RenderingManager ()
{
  CloseFunc();
}