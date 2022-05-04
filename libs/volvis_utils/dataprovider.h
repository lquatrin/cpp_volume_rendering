#ifndef VOL_VIS_UTILS_DATA_PROVIDER_H
#define VOL_VIS_UTILS_DATA_PROVIDER_H

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>

namespace vis
{
  class StructuredGridVolume;
  class TransferFunction;

  class DataProvider
  {
  public:
    DataProvider ();
    ~DataProvider ();

    // Structured Grids
    void ClearStructuredGridFileList ();
    bool SetStructuredGridFileList (std::string file_path, std::string file_name);
    int GetNumberOfStructuredGrids ();
    int FindStructuredGridName (std::string name);
    std::vector<std::string>& GetStructuredGridNameList ();

    // Load Structured Grid
    vis::StructuredGridVolume* LoadStructuredGrid (unsigned int id);
    vis::StructuredGridVolume* LoadStructuredGridFromRawFile (std::string name, std::string raw_file_path, glm::ivec3 resolution, glm::vec3 scale, int bytes_per_value);

    // Transfer Functions
    void ClearTransferFunctionFileList ();
    bool SetTransferFunctionFileList (std::string file_path, std::string file_name);
    int GetNumberOfTransferFunctions ();
    std::vector<std::string>& GetTransferFunctionNameList ();

    vis::TransferFunction* LoadTransferFunction (unsigned int id);

  protected:
  private:
    std::string GetExtension (std::string filename);
    int Split (std::vector<std::string>& tags, std::string line, std::string separator);

    bool GetCSVStructuredGridTags (std::vector<std::string>& tags, unsigned int id);
    int GetBytesPerValueByType (std::string value_type);

    enum DATA_PROVIDER_FILE_TYPE_EXT : unsigned int {
      NO_EXTENSION = 0,
      CSV = 1,
    };

    // Auxiliar class for CSV reader
    class CSVstructuredgrid {
    public:
      CSVstructuredgrid () {}
      ~CSVstructuredgrid () {}
    
      std::pair<bool, std::string> GetTagValue (std::vector<std::string>& values, std::string tag);
  
      std::string separator;
      std::vector<std::string> tags;
    protected:
    private:
    };
    std::unique_ptr<CSVstructuredgrid> m_csv_str_grid;

    // Structured Grids
    std::string m_path_list_structured_grid;
    std::string m_file_list_structured_grid;
    DATA_PROVIDER_FILE_TYPE_EXT m_file_list_structured_grid_type;

    std::vector<std::string> m_uiname_list_structured_grid;

    // Transfer Functions
    std::string m_path_list_transfer_function;
    std::string m_file_list_transfer_function;
    DATA_PROVIDER_FILE_TYPE_EXT m_file_list_transfer_function_type;

    std::vector<std::string> m_uiname_list_transfer_function;
  };
}

#endif