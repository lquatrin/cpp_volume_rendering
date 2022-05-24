/**
 * datamanager.cpp
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#include <volvis_utils/datamanager.h>

#include <fstream>
#include <gl_utils/computeshader.h>
#include <vis_utils/defines.h>
#include <volvis_utils/utils.h>


#include <volvis_utils/reader.h>

namespace vis
{
  DataManager::DataManager ()
    : curr_vol_data_type(vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
    , use_specific_lookup_data_shader(false)
    , curr_vr_volume(nullptr)
    , curr_uns_grid_volume(nullptr)
    , curr_vr_transferfunction(nullptr)
    , curr_volume_index(0)
    , curr_transferfunction_index(0)
    , curr_gradient_comp_model(DataManager::STRUCTURED_GRADIENT_TYPE::NONE_GRADIENT)
    , curr_gl_tex_structured_volume(nullptr)
    , curr_gl_tex_structured_gradient(nullptr)
  {
    m_path_to_data = "";
#ifdef USE_DATA_PROVIDER
    m_data_provider = std::make_unique<DataProvider>();
#else
    // structured, unstructured and transfer function list...
    stored_structured_datasets.clear();
    stored_transfer_functions.clear();

    ui_dataset_names.clear();
    ui_transferf_names.clear();
#endif
  }

  DataManager::~DataManager ()
  {
    DeleteVolumeData();
    DeleteTransferFunctionData();
  }

  void DataManager::SetPathToData (std::string s_path_to_data)
  {
    m_path_to_data = s_path_to_data;
  }

  vis::GRID_VOLUME_DATA_TYPE DataManager::GetInputVolumeDataType ()
  {
    return curr_vol_data_type;
  }
  
  const char* DataManager::GetStrVolumeDataType ()
  {
    if (GetInputVolumeDataType() == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
      return "Structured";
    else if (GetInputVolumeDataType() == vis::GRID_VOLUME_DATA_TYPE::UNSTRUCTURED)
      return "Unstructured";
    return "None";
  }

  void DataManager::ReadData ()
  {
#ifdef USE_DATA_PROVIDER
    m_data_provider->ClearStructuredGridFileList();
    m_data_provider->ClearTransferFunctionFileList();

    if (curr_vol_data_type == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED) {
      //m_data_provider->SetStructuredGridFileList(m_path_to_data, "#list_structured_datasets");
      m_data_provider->SetStructuredGridFileList(m_path_to_data, "volume_list.csv");
      curr_volume_index = 0;
    }
#else
    stored_structured_datasets.clear();
    stored_transfer_functions.clear();

    ui_dataset_names.clear();
    ui_transferf_names.clear();

    if (curr_vol_data_type == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
      ReadStructuredDatasetsFromRes();
#endif

    ReadTransferFunctionsFromRes();

    if (curr_vol_data_type == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
    {
      GenerateStructuredVolumeTexture();
    }

    vis::TransferFunctionReader tfr;
    curr_vr_transferfunction = tfr.ReadTransferFunction(stored_transfer_functions[GetCurrentTransferFunctionIndex()].path);
    curr_vr_transferfunction->SetName(stored_transfer_functions[GetCurrentTransferFunctionIndex()].name);
  }

  int DataManager::GetNumberOfStructuredDatasets ()
  {
#ifdef USE_DATA_PROVIDER
    return m_data_provider->GetNumberOfStructuredGrids();
#else
    return stored_structured_datasets.size();
#endif
  }

  int DataManager::GetCurrentVolumeIndex ()
  {
    return curr_volume_index;
  }

  std::string DataManager::GetCurrentVolumeName ()
  {
    if (GetInputVolumeDataType() == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED) 
    {
#ifdef USE_DATA_PROVIDER
      return m_data_provider->GetStructuredGridNameList()[GetCurrentVolumeIndex()];
#else
      return stored_structured_datasets[GetCurrentVolumeIndex()].name;
#endif
    }
    return "";
  }

  vis::GridVolume* DataManager::GetCurrentGridVolume ()
  {
    if (GetInputVolumeDataType() == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
      return curr_vr_volume;
    if (GetInputVolumeDataType() == vis::GRID_VOLUME_DATA_TYPE::UNSTRUCTURED)
      return curr_uns_grid_volume;
  }

  vis::StructuredGridVolume* DataManager::GetCurrentStructuredVolume ()
  {
    return curr_vr_volume;
  }

  vis::UnstructuredGridVolume* DataManager::GetCurrentUnstructuredVolume ()
  {
    return curr_uns_grid_volume;
  }
  
  vis::TransferFunction* DataManager::GetCurrentTransferFunction ()
  {
    return curr_vr_transferfunction;
  }

  void DataManager::AddDataLookUpShader (gl::PipelineShader* ext_shader)
  {
    if (GetInputVolumeDataType() == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
    {
      //return stored_structured_datasets[GetCurrentVolumeIndex()].name;
    }
  }

  void DataManager::AddDataLookUpShader (gl::ComputeShader* ext_shader)
  {
    if (GetInputVolumeDataType() == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
    {
      if (use_specific_lookup_data_shader)
      {
        ext_shader->AddShaderFile(std::string(vis::Utils::GetShaderPath() + "_data_lookup/structured_8bits.comp"));
      }
      else
      {
        ext_shader->AddShaderFile(std::string(vis::Utils::GetShaderPath() + "_data_lookup/structured_half_float.comp"));
      }
    }
  }

  int DataManager::GetCurrentTransferFunctionIndex ()
  {
    return curr_transferfunction_index;
  }
  
  std::string DataManager::GetCurrentTransferFunctionName ()
  {
    return stored_transfer_functions[GetCurrentTransferFunctionIndex()].name;
  }

  gl::Texture3D* DataManager::GetCurrentVolumeTexture ()
  {
    return curr_gl_tex_structured_volume;
  }

  gl::Texture3D* DataManager::GetCurrentGradientTexture ()
  {
    return curr_gl_tex_structured_gradient;
  }

  std::vector<std::string>& DataManager::GetUINameTransferFunctionList ()
  {
#ifdef USE_DATA_PROVIDER
    return m_data_provider->GetTransferFunctionNameList();
#else
    return ui_transferf_names;
#endif
  }

  ////////////////////////////////////////////////////////////////////////
  // Private Methods
  ////////////////////////////////////////////////////////////////////////
  void DataManager::DeleteVolumeData ()
  {
    if (curr_vr_volume) delete curr_vr_volume;
    curr_vr_volume = nullptr;

    if (curr_gl_tex_structured_volume) delete curr_gl_tex_structured_volume;
    curr_gl_tex_structured_volume = nullptr;

    DeleteGradientData();
  }

  void DataManager::DeleteGradientData ()
  {
    if (curr_gl_tex_structured_gradient) delete curr_gl_tex_structured_gradient;
    curr_gl_tex_structured_gradient = nullptr;
  }

  void DataManager::DeleteTransferFunctionData ()
  {
    if (curr_vr_transferfunction) delete curr_vr_transferfunction;
    curr_vr_transferfunction = nullptr;
  }

#ifndef USE_DATA_PROVIDER
  void DataManager::ReadStructuredDatasetsFromRes ()
  {
    std::string line;
    std::string model_read_filename = m_path_to_data;
    model_read_filename.append("/#list_structured_datasets");
    std::ifstream f_openmodels(model_read_filename);

    if (!f_openmodels.is_open())
    {
      std::cout << "Error: Unable to read vol rendering datasets." << std::endl;
      exit(EXIT_FAILURE);
    }

    std::cout << "Reading structured datasets..." << std::endl;
    while (!f_openmodels.eof())
    {
      line.clear();
      std::getline(f_openmodels, line);

      int start_path = line.find_first_of("<") + 1;
      int end_path = line.find_first_of(">");

      int start_name = line.find_last_of("<") + 1;
      int end_name = line.find_last_of(">");

      std::string path_to_model = line.substr(start_path, end_path - start_path);
      std::string model_name = line.substr(start_name, end_name - start_name);

      stored_structured_datasets.push_back(DataReference(path_to_model, model_name.empty() ? path_to_model : model_name, m_path_to_data));
    }
    f_openmodels.close();

    for (int i = 0; i < stored_structured_datasets.size(); i++)
    {
      std::cout << i << ": " << stored_structured_datasets[i].name << std::endl;
      ui_dataset_names.push_back(stored_structured_datasets[i].name);
    }

    curr_volume_index = 0;
  }

  void DataManager::ReadTransferFunctionsFromRes ()
  {
    std::string line;
    std::string tfunc_read_filename = m_path_to_data;
    tfunc_read_filename.append("/#list_transfer_functions");
    std::ifstream f_opentransferfunctions(tfunc_read_filename);

    if (!f_opentransferfunctions.is_open())
    {
      std::cout << "Error: Unable to read vol rendering transfer functions." << std::endl;
      exit(EXIT_FAILURE);
    }

    std::cout << "Reading Transfer Functions...";
    while (!f_opentransferfunctions.eof())
    {
      line.clear();
      std::getline(f_opentransferfunctions, line);

      int start_path = line.find_first_of("<") + 1;
      int end_path = line.find_first_of(">");

      int start_name = line.find_last_of("<") + 1;
      int end_name = line.find_last_of(">");

      stored_transfer_functions.push_back(DataReference(
        line.substr(start_path, end_path - start_path),
        line.substr(start_name, end_name - start_name),
        m_path_to_data
      ));
    }
    f_opentransferfunctions.close();

    for (int i = 0; i < stored_transfer_functions.size(); i++)
      ui_transferf_names.push_back(stored_transfer_functions[i].name);
  }
#endif

  bool DataManager::GenerateStructuredVolumeTexture ()
  {
    // Read Volume
#ifdef USE_DATA_PROVIDER
    curr_vr_volume = m_data_provider->LoadStructuredGrid(GetCurrentVolumeIndex());
#else
    vis::VolumeReader vr;
    curr_vr_volume = vr.ReadStructuredVolume(stored_structured_datasets[GetCurrentVolumeIndex()].path);
    curr_vr_volume->SetName(stored_structured_datasets[GetCurrentVolumeIndex()].name); 
#endif

    // Generate Volume Texture
    curr_gl_tex_structured_volume = vis::GenerateRTexture(curr_vr_volume, 0, 0, 0, curr_vr_volume->GetWidth(),
      curr_vr_volume->GetHeight(), curr_vr_volume->GetDepth());

    // Generate gradient, if enabled
    GenerateStructuredGradientTexture();

    return true;
  }

  bool DataManager::GenerateStructuredGradientTexture ()
  {
    if (curr_gradient_comp_model == STRUCTURED_GRADIENT_TYPE::SOBEL_FELDMAN_FILTER)
    {
      curr_gl_tex_structured_gradient = vis::GenerateSobelFeldmanGradientTexture(curr_vr_volume);
    }
    else if (curr_gradient_comp_model == STRUCTURED_GRADIENT_TYPE::FINITE_DIFERENCES)
    {
      curr_gl_tex_structured_gradient = vis::GenerateGradientTexture(curr_vr_volume);
    }
    else if (curr_gradient_comp_model == STRUCTURED_GRADIENT_TYPE::COMPUTE_SHADER_SOBEL)
    {
      curr_gl_tex_structured_gradient = GenerateGradientWithComputeShader();
    }
    else
    {
      curr_gl_tex_structured_gradient = nullptr;
      return false;
    }
    return true;
  }

  bool DataManager::PreviousVolume ()
  {
    if (curr_vol_data_type == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
    {
      if (GetCurrentVolumeIndex() > 0)
      {
        curr_volume_index -= 1;
        DeleteVolumeData();
  
        GenerateStructuredVolumeTexture();

        return true;
      }
    }
    return false;
  }
  
  bool DataManager::NextVolume ()
  {
    if (curr_vol_data_type == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
    {
      if (curr_volume_index + 1 < GetNumberOfStructuredDatasets())
      {
        curr_volume_index += 1;
        DeleteVolumeData();
  
        GenerateStructuredVolumeTexture();
  
        return true;
      }
    }
    return false;
  }

  bool DataManager::SetVolume (std::string name)
  {
    if (curr_vol_data_type == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
    {
#ifdef USE_DATA_PROVIDER
      int new_volume_id = m_data_provider->FindStructuredGridName(name);
      if (new_volume_id != -1) {
        curr_volume_index = new_volume_id;
        DeleteVolumeData();
        GenerateStructuredVolumeTexture();
        return true;
      }
#else
      for (int i = 0; i < stored_structured_datasets.size(); i++) 
      {
        if (stored_structured_datasets[i].name.compare(name) == 0)
        {
          curr_volume_index = i;
          DeleteVolumeData();

          GenerateStructuredVolumeTexture();
          return true;
        }
      }
#endif
    }
    return false;
  }

  bool DataManager::SetCurrentInputVolume (int id)
  {
    if (curr_vol_data_type == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
    {
      if (id < GetNumberOfStructuredDatasets())
      {
        curr_volume_index = id;
        DeleteVolumeData();
        GenerateStructuredVolumeTexture();
        return true;
      }
    }
    return false;
  }
    
  bool DataManager::PreviousTransferFunction ()
  {
    if (curr_transferfunction_index > 0)
    {
      curr_transferfunction_index -= 1;
      DeleteTransferFunctionData();
  
      vis::TransferFunctionReader tfr;
      curr_vr_transferfunction = tfr.ReadTransferFunction(stored_transfer_functions[curr_transferfunction_index].path);
      curr_vr_transferfunction->SetName(stored_transfer_functions[curr_transferfunction_index].name);
  
      return true;
    }
    return false;
  }
  
  bool DataManager::NextTransferFunction ()
  {
    if (curr_transferfunction_index + 1 < stored_transfer_functions.size())
    {
      curr_transferfunction_index += 1;
      DeleteTransferFunctionData();
  
      vis::TransferFunctionReader tfr;
      curr_vr_transferfunction = tfr.ReadTransferFunction(stored_transfer_functions[curr_transferfunction_index].path);
      curr_vr_transferfunction->SetName(stored_transfer_functions[curr_transferfunction_index].name);
  
      return true;
    }
    return false;
  }
  
  bool DataManager::SetTransferFunction (std::string name)
  {
    for (int i = 0; i < stored_transfer_functions.size(); i++)
    {
      if (stored_transfer_functions[i].name.compare(name) == 0)
      {
        curr_transferfunction_index = i;
        DeleteTransferFunctionData();

        vis::TransferFunctionReader tfr;
        curr_vr_transferfunction = tfr.ReadTransferFunction(stored_transfer_functions[curr_transferfunction_index].path);
        curr_vr_transferfunction->SetName(stored_transfer_functions[curr_transferfunction_index].name);

        return true;
      }
    }
    return false;
  }

  bool DataManager::SetCurrentTransferFunction (int id)
  {
    if (id < stored_transfer_functions.size())
    {
      curr_transferfunction_index = id;
      DeleteTransferFunctionData();

      vis::TransferFunctionReader tfr;
      curr_vr_transferfunction = tfr.ReadTransferFunction(stored_transfer_functions[curr_transferfunction_index].path);
      curr_vr_transferfunction->SetName(stored_transfer_functions[curr_transferfunction_index].name);

      return true;
    }
    return false;
  }
  
  bool DataManager::UpdateStructuredGradientTexture ()
  {
    DeleteGradientData();
    return GenerateStructuredGradientTexture();
  }
  
  int DataManager::GetCurrentGradientGenerationTypeID ()
  {
    if (curr_vol_data_type == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
    {
      if (curr_gradient_comp_model == STRUCTURED_GRADIENT_TYPE::SOBEL_FELDMAN_FILTER)
      {
        return 0;
      }
      else if (curr_gradient_comp_model == STRUCTURED_GRADIENT_TYPE::FINITE_DIFERENCES)
      {
        return 1;
      }
      else if (curr_gradient_comp_model == STRUCTURED_GRADIENT_TYPE::COMPUTE_SHADER_SOBEL)
      {
        return 2;
      }
      else if (curr_gradient_comp_model == STRUCTURED_GRADIENT_TYPE::NONE_GRADIENT)
      {
        return 3;
      }
    }
    return -1;
  }

  int DataManager::GetGradientIndex (DataManager::STRUCTURED_GRADIENT_TYPE sgt)
  {
    if (curr_vol_data_type == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
    {
      if (sgt == STRUCTURED_GRADIENT_TYPE::SOBEL_FELDMAN_FILTER)
        return 0;
      else if (sgt == STRUCTURED_GRADIENT_TYPE::FINITE_DIFERENCES)
        return 1;
      else if (sgt == STRUCTURED_GRADIENT_TYPE::COMPUTE_SHADER_SOBEL)
        return 2;
      else
        return 3;
    }
    return -1;
  }

  bool DataManager::SetCurrentGradient (int idx)
  {
    STRUCTURED_GRADIENT_TYPE sgt = STRUCTURED_GRADIENT_TYPE::NONE_GRADIENT;
    if (curr_vol_data_type == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
    {
      if (idx == 0)
        sgt = STRUCTURED_GRADIENT_TYPE::SOBEL_FELDMAN_FILTER;
      else if (idx == 1)
        sgt = STRUCTURED_GRADIENT_TYPE::FINITE_DIFERENCES;
      else if (idx == 2)
        sgt = STRUCTURED_GRADIENT_TYPE::COMPUTE_SHADER_SOBEL;
    }

    bool ret = !(sgt == curr_gradient_comp_model);
    if (ret) curr_gradient_comp_model = sgt;
  
    return ret;
  }

  std::string DataManager::GetGradientName (DataManager::STRUCTURED_GRADIENT_TYPE sgt)
  {
    if (curr_vol_data_type == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
    {
      if (sgt == STRUCTURED_GRADIENT_TYPE::SOBEL_FELDMAN_FILTER)
      {
        return "Sobel-Feldman";
      }
      else if (sgt == STRUCTURED_GRADIENT_TYPE::FINITE_DIFERENCES)
      {
        return "Finite Diferences";
      }
      else if (sgt == STRUCTURED_GRADIENT_TYPE::COMPUTE_SHADER_SOBEL)
      {
        return "Sobel-Feldman (Compute Shader)";
      }
    }
    return "None";
  }

  std::string DataManager::CurrentGradientName ()
  {
    if (curr_vol_data_type == vis::GRID_VOLUME_DATA_TYPE::STRUCTURED)
    {
      if (curr_gradient_comp_model == STRUCTURED_GRADIENT_TYPE::SOBEL_FELDMAN_FILTER)
      {
        return "Sobel-Feldman";
      }
      else if (curr_gradient_comp_model == STRUCTURED_GRADIENT_TYPE::FINITE_DIFERENCES)
      {
        return "Finite Diferences";
      }
      else if (curr_gradient_comp_model == STRUCTURED_GRADIENT_TYPE::COMPUTE_SHADER_SOBEL)
      {
        return "Sobel-Feldman (Compute Shader)";
      }
    }
    return "NULL";
  }
  
  std::vector<std::string> DataManager::GetGradientGenerationTypeStrList ()
  {
    std::vector<std::string> vlist;
    vlist.push_back(GetGradientName(STRUCTURED_GRADIENT_TYPE::SOBEL_FELDMAN_FILTER));
    vlist.push_back(GetGradientName(STRUCTURED_GRADIENT_TYPE::FINITE_DIFERENCES));
    vlist.push_back(GetGradientName(STRUCTURED_GRADIENT_TYPE::COMPUTE_SHADER_SOBEL));
    vlist.push_back(GetGradientName(STRUCTURED_GRADIENT_TYPE::NONE_GRADIENT));
    return vlist;
  }

  std::vector<std::string>& DataManager::GetUINameDatasetList ()
  {
#ifdef USE_DATA_PROVIDER
    return m_data_provider->GetStructuredGridNameList();
#else
    return ui_dataset_names;
#endif
  }

  gl::Texture3D* DataManager::GenerateGradientWithComputeShader ()
  {
    // Get Current Volume
    vis::StructuredGridVolume* vol = GetCurrentStructuredVolume();

    // Initialize compute shader
    gl::ComputeShader* cpshader = new gl::ComputeShader();
    cpshader->SetShaderFile(MAKE_STR(CMAKE_VOLVIS_UTILS_PATH_TO_SHADER)"/_gradient_shading/sobelfeldman_generator.comp");
    cpshader->LoadAndLink();
    cpshader->Bind();
    
    // Initialize 1-channel textures
    gl::Texture3D* tex3d[3];
    for (int i = 0; i < 3; i++)
    {
      tex3d[i] = new gl::Texture3D(vol->GetWidth(), vol->GetHeight(), vol->GetDepth());
      tex3d[i]->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
      tex3d[i]->SetData(NULL, GL_R16F, GL_RED, GL_FLOAT);
    
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_3D, tex3d[i]->GetTextureID());
    }
    
    // Bind 1-channel textures
    for (int i = 0; i < 3; i++)
      glBindImageTexture(i, tex3d[i]->GetTextureID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R16F);
    
    // Bind volume and volume dimensions
    cpshader->SetUniformTexture3D("TexVolume", GetCurrentVolumeTexture()->GetTextureID(), 3);
    cpshader->BindUniform("TexVolume");
    
    cpshader->SetUniform("VolumeDimensions", glm::vec3(vol->GetWidth(), vol->GetHeight(), vol->GetDepth()));
    cpshader->BindUniform("VolumeDimensions");
    
    // Compute the number of groups and dispatch
    cpshader->RecomputeNumberOfGroups(vol->GetWidth(), vol->GetHeight(), vol->GetDepth());
    cpshader->Dispatch();
    
    // Delete compute shader
    cpshader->Unbind();
    delete cpshader;
    
    // Initialize Red Data
    GLfloat* red_data = new GLfloat[vol->GetWidth() * vol->GetHeight() * vol->GetDepth()];
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, tex3d[0]->GetTextureID());
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, red_data);
    glBindTexture(GL_TEXTURE_3D, 0);
    
    // Initialize Green Data
    GLfloat* green_data = new GLfloat[vol->GetWidth() * vol->GetHeight() * vol->GetDepth()];
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, tex3d[1]->GetTextureID());
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, green_data);
    glBindTexture(GL_TEXTURE_3D, 0);
    
    // Initialize Blue Data
    GLfloat* blue_data = new GLfloat[vol->GetWidth() * vol->GetHeight() * vol->GetDepth()];
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, tex3d[2]->GetTextureID());
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, blue_data);
    glBindTexture(GL_TEXTURE_3D, 0);
    
    // Delete 1-channel textures
    for (int i = 0; i < 3; i++)
      delete tex3d[i];
    
    // Initialize RGB Gradient Values Array
    GLfloat* gradient_values = new GLfloat[vol->GetWidth() * vol->GetHeight() * vol->GetDepth() * 3];
    for (int i = 0; i < vol->GetWidth() * vol->GetHeight() * vol->GetDepth(); i++)
    {
      gradient_values[i * 3 + 0] = red_data[i];
      gradient_values[i * 3 + 1] = green_data[i];
      gradient_values[i * 3 + 2] = blue_data[i];
    }
    
    // Delete Red Data
    delete[] red_data;
    
    // Delete Green Data
    delete[] green_data;
    
    // Delete Blue Data
    delete[] blue_data;
    
    // Generate a new terxture and set the gradient values [red, green, blue]
    gl::Texture3D* tex3d_gradient = new gl::Texture3D(vol->GetWidth(), vol->GetHeight(), vol->GetDepth());
    tex3d_gradient->GenerateTexture(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    tex3d_gradient->SetData((GLvoid*)gradient_values, GL_RGB16F, GL_RGB, GL_FLOAT);
    
    // Delete RGB Gradient Values Array
    delete[] gradient_values;
  
    return tex3d_gradient;
  }
}