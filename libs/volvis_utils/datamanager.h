/**
 * datamanager.h
 *
 * Class that encapsulates datasets and transfer functions
 * . Must define a path to resources
 *
 * Receives a path that contains 3 files with data lists:
 * . volrend_structured_datasets
 * . volrend_unstructured_datasets
 * . volrend_transfun
 *
 * All files are written as:
 * <path to file 1 from "path to resources"> <name of file 1 to be displayed in UI>
 * <path to file 2 from "path to resources"> <name of file 2 to be displayed in UI>
 * <path to file 3 from "path to resources"> <name of file 3 to be displayed in UI>
 * <path to file 4 from "path to resources"> <name of file 4 to be displayed in UI>
 * <path to file 5 from "path to resources"> <name of file 5 to be displayed in UI>
 * ... until eof
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef VOL_VIS_UTILS_DATA_MANAGER_H
#define VOL_VIS_UTILS_DATA_MANAGER_H

#include <iostream>

#include <volvis_utils/dataprovider.h>
#include <volvis_utils/gridvolume.h>
#include <volvis_utils/structuredgridvolume.h>
#include <volvis_utils/unstructuredgridvolume.h>
#include <volvis_utils/transferfunction.h>
#include <volvis_utils/reader.h>

#include <gl_utils/texture3d.h>
#include <gl_utils/texture1d.h>
#include <gl_utils/computeshader.h>
#include <gl_utils/pipelineshader.h>

//#define USE_DATA_PROVIDER

namespace vis
{
  class DataReference
  {
  public:
    DataReference () {}
    DataReference (std::string _path, std::string _name = "", std::string PATH_TO_RESOURCES = "")
    {
      path = std::string(PATH_TO_RESOURCES);
      path.append(_path);
      name = _name;
      if (name == "")
        name = path.substr(path.find_last_of('/') + 1);
    }

    std::string path;
    std::string name;
  };

  class DataManager
  {
  public:
    enum STRUCTURED_GRADIENT_TYPE : unsigned int {
      SOBEL_FELDMAN_FILTER = 0,
      FINITE_DIFERENCES    = 1,
      COMPUTE_SHADER_SOBEL = 2,
      NONE_GRADIENT        = 3
    };

    DataManager ();
    ~DataManager ();

    void SetPathToData (std::string s_path_to_data);

    vis::GRID_VOLUME_DATA_TYPE GetInputVolumeDataType ();
    const char* GetStrVolumeDataType ();

    void ReadData ();

    // Read data
    int GetNumberOfStructuredDatasets ();
    int GetCurrentVolumeIndex ();
    std::string GetCurrentVolumeName ();
    vis::GridVolume* GetCurrentGridVolume ();
    vis::StructuredGridVolume* GetCurrentStructuredVolume ();
    vis::UnstructuredGridVolume* GetCurrentUnstructuredVolume ();

    int GetCurrentTransferFunctionIndex ();
    std::string GetCurrentTransferFunctionName ();
    vis::TransferFunction* GetCurrentTransferFunction ();

    void AddDataLookUpShader (gl::PipelineShader* ext_shader);
    void AddDataLookUpShader (gl::ComputeShader* ext_shader);

    // Processed data
    gl::Texture3D* GetCurrentVolumeTexture ();

    gl::Texture3D* GetCurrentGradientTexture ();

    bool PreviousVolume ();
    bool NextVolume ();
    bool SetVolume (std::string name);
    bool SetCurrentInputVolume (int id);

    bool PreviousTransferFunction ();
    bool NextTransferFunction ();
    bool SetTransferFunction (std::string name);
    bool SetCurrentTransferFunction (int id);

    bool UpdateStructuredGradientTexture ();
    int GetCurrentGradientGenerationTypeID ();
    int GetGradientIndex (DataManager::STRUCTURED_GRADIENT_TYPE sgt);
    bool SetCurrentGradient (int idx);
    std::string GetGradientName (DataManager::STRUCTURED_GRADIENT_TYPE sgt);
    std::string CurrentGradientName ();
    std::vector<std::string> GetGradientGenerationTypeStrList ();
 
    std::vector<std::string>& GetUINameDatasetList ();
    std::vector<std::string>& GetUINameTransferFunctionList ();

    void DeleteVolumeData ();
    void DeleteTransferFunctionData ();
    void DeleteGradientData ();
  protected:
#ifndef USE_DATA_PROVIDER
    void ReadStructuredDatasetsFromRes ();
    void ReadTransferFunctionsFromRes ();
#endif

    bool GenerateStructuredVolumeTexture ();
    bool GenerateStructuredGradientTexture ();

    // Compute Shaders doesn't support rgb textures, so
    //  we bind 3 r textures, set the data in the shader,
    //  then we group into a single array and set into a
    //  new rgb texture using glTexImage3D 
    gl::Texture3D* GenerateGradientWithComputeShader ();
    
    vis::GRID_VOLUME_DATA_TYPE curr_vol_data_type;
    bool use_specific_lookup_data_shader;

    // structured datasets
    vis::StructuredGridVolume* curr_vr_volume;
    gl::Texture3D* curr_gl_tex_structured_volume;

    // unstructured datasets
    vis::UnstructuredGridVolume* curr_uns_grid_volume;

    // transfer function
    vis::TransferFunction* curr_vr_transferfunction;

    int curr_volume_index;
    int curr_transferfunction_index;

    STRUCTURED_GRADIENT_TYPE curr_gradient_comp_model;
    gl::Texture3D* curr_gl_tex_structured_gradient;

    std::string m_path_to_data;
    
#ifdef USE_DATA_PROVIDER
    std::unique_ptr<DataProvider> m_data_provider;
#else
    // structured, unstructured and transfer function list...
    std::vector<DataReference> stored_structured_datasets;
    std::vector<DataReference> stored_transfer_functions;

    std::vector<std::string> ui_dataset_names;
    std::vector<std::string> ui_transferf_names;
#endif
  private:

  };
}

#endif