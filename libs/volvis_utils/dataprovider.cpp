#include <volvis_utils/dataprovider.h>

#include <fstream>
#include <sstream>
#include <gl_utils/computeshader.h>
#include <vis_utils/defines.h>
#include <volvis_utils/utils.h>

#include <volvis_utils/reader.h>

namespace vis
{
  DataProvider::DataProvider ()
  {
  }
  
  DataProvider::~DataProvider ()
  {
  }

  // Structured Grids
  void DataProvider::ClearStructuredGridFileList ()
  {
    m_uiname_list_structured_grid.clear();
  }

  bool DataProvider::SetStructuredGridFileList (std::string file_path, std::string file_name)
  {
    m_path_list_structured_grid = file_path;
    m_file_list_structured_grid = file_name;
    m_uiname_list_structured_grid.clear();

    std::cout << "Reading structured grids..." << std::endl;
    if (std::string::npos == m_file_list_structured_grid.find(".")) {
      m_file_list_structured_grid_type = DATA_PROVIDER_FILE_TYPE_EXT::NO_EXTENSION;
    
      std::string str_file = m_path_list_structured_grid + m_file_list_structured_grid;
      
      std::ifstream f_open_file(str_file);
      if (!f_open_file.is_open()) {
        std::cout << "Error: Unable to read structured grid file list." << std::endl;
        exit(EXIT_FAILURE);
      }
      
      while (!f_open_file.eof()) {
        std::string line;
        std::getline(f_open_file, line);
      
        int start_path = line.find_first_of("<") + 1;
        int end_path = line.find_first_of(">");
      
        int start_name = line.find_last_of("<") + 1;
        int end_name = line.find_last_of(">");

        std::string model_name = line.substr(start_name, end_name - start_name);

        std::cout << ". " << model_name << std::endl;

        if (model_name.empty()) {
          std::string path_model = line.substr(start_path, end_path - start_path);
          m_uiname_list_structured_grid.push_back(path_model);
        }
        else {
          m_uiname_list_structured_grid.push_back(model_name);
        }
      }
      f_open_file.close();
    }
    else {
      std::string extension = GetExtension(m_file_list_structured_grid);
      if (extension.compare("csv") == 0) {
        m_file_list_structured_grid_type = DATA_PROVIDER_FILE_TYPE_EXT::CSV;

        std::string str_file = m_path_list_structured_grid + m_file_list_structured_grid;

        std::ifstream f_open_file(str_file);
        if (!f_open_file.is_open()) {
          std::cout << "Error: Unable to read structured grid file list." << std::endl;
          exit(EXIT_FAILURE);
        }

        std::string first_line;
        std::getline(f_open_file, first_line);

        m_csv_str_grid.reset(new CSVstructuredgrid());

        std::string header;
        if (first_line.substr(0, 3).compare("sep") == 0) {
          m_csv_str_grid->separator = first_line.substr(first_line.size()-1, 1);
          std::getline(f_open_file, header);
        }
        else {
          header = first_line;
        }

        Split(m_csv_str_grid->tags, header, m_csv_str_grid->separator);

        while (!f_open_file.eof()) {
          std::string line;
          std::getline(f_open_file, line);

          std::vector<std::string> grid_tags;
          Split(grid_tags, line, m_csv_str_grid->separator);
          
          std::pair<bool, std::string> str_file = m_csv_str_grid->GetTagValue(grid_tags, "Name");

          m_uiname_list_structured_grid.push_back(str_file.second);
        }
        f_open_file.close();
      }
    }
    return true;
  }

  int DataProvider::GetNumberOfStructuredGrids ()
  {
    return (int)m_uiname_list_structured_grid.size();
  }

  int DataProvider::FindStructuredGridName (std::string name)
  {
    for (int i = 0; i < m_uiname_list_structured_grid.size(); i++) {
      if (m_uiname_list_structured_grid[i].compare(name) == 0)
        return i;
    }
    return -1;
  }

  std::vector<std::string>& DataProvider::GetStructuredGridNameList ()
  {
    return m_uiname_list_structured_grid;
  }

  vis::StructuredGridVolume* DataProvider::LoadStructuredGrid (unsigned int id)
  {
    vis::StructuredGridVolume* sg = nullptr;

    if (m_file_list_structured_grid_type == DATA_PROVIDER_FILE_TYPE_EXT::NO_EXTENSION) {
      std::string str_file = m_path_list_structured_grid + m_file_list_structured_grid;

      std::ifstream f_open_file(str_file);
      if (!f_open_file.is_open()) {
        std::cout << "Error: Unable to read structured grid file list." << std::endl;
        exit(EXIT_FAILURE);
      }

      int volume_id = 0;
      while (!f_open_file.eof()) {
        std::string line;
        std::getline(f_open_file, line);
        if (volume_id == id) {

          int start_path = line.find_first_of("<") + 1;
          int end_path = line.find_first_of(">");

          std::string path_file = line.substr(start_path, end_path - start_path);

          vis::VolumeReader vr;
          sg = vr.ReadStructuredVolume(m_path_list_structured_grid + "/" + path_file);
          f_open_file.close();
          
          return sg;
        }
        volume_id++;
      }
      f_open_file.close();
    }
    else if (m_file_list_structured_grid_type == DATA_PROVIDER_FILE_TYPE_EXT::CSV) {
      std::vector<std::string> values;
      GetCSVStructuredGridTags(values, id);

      std::pair<bool, std::string> str_source = m_csv_str_grid->GetTagValue(values, "Source");
      std::pair<bool, std::string> str_file = m_csv_str_grid->GetTagValue(values, "File");

      std::pair<bool, std::string> str_size = m_csv_str_grid->GetTagValue(values, "Size");
      std::vector<int> sizes;
      {
        std::istringstream i_sizes(str_size.second);
        std::string split_values;
        while (std::getline(i_sizes, split_values, 'x')) {
          sizes.push_back(std::stoi(split_values));
        }
      }

      std::pair<bool, std::string> str_spacing = m_csv_str_grid->GetTagValue(values, "Spacing");
      std::vector<float> spacings;
      {
        std::istringstream i_sizes(str_spacing.second);
        std::string split_values;
        while (std::getline(i_sizes, split_values, 'x')) {
          spacings.push_back(std::stof(split_values));
        }
      }

      std::pair<bool, std::string> str_type = m_csv_str_grid->GetTagValue(values, "Type");
      std::string data_file_path(m_path_list_structured_grid + "/" + str_source.second + "/" + str_file.second);
      
      if (GetExtension(data_file_path).compare("raw") == 0) {
        return LoadStructuredGridFromRawFile(
          m_uiname_list_structured_grid[id], 
          data_file_path, 
          glm::ivec3(sizes[0], sizes[1], sizes[2]),
          glm::vec3(spacings[0], spacings[1], spacings[2]),
          GetBytesPerValueByType(str_type.second)
        );
      }
      else {
        vis::VolumeReader vr;
        return vr.ReadStructuredVolume(data_file_path);
      }
    }
    return nullptr;
  }

  vis::StructuredGridVolume* DataProvider::LoadStructuredGridFromRawFile (std::string name, std::string raw_file_path, glm::ivec3 resolution, glm::vec3 scale, int bytes_per_value)
  {
    vis::StructuredGridVolume* sg = new StructuredGridVolume(name, resolution.x, resolution.y, resolution.z);
    sg->SetScale(scale.x, scale.y, scale.z);

    vis::VolumeReader vr;
    vr.SetArrayDataFromRawFile(raw_file_path, sg, bytes_per_value);

    return sg;
  }

  // Transfer Functions
  void DataProvider::ClearTransferFunctionFileList ()
  {
    m_uiname_list_transfer_function.clear();
  }

  bool DataProvider::SetTransferFunctionFileList (std::string file_path, std::string file_name)
  {
    m_path_list_transfer_function = file_path;
    m_file_list_transfer_function = file_name;
    m_uiname_list_transfer_function.clear();

    std::cout << "Reading structured grids..." << std::endl;
    if (std::string::npos == m_file_list_structured_grid.find(".")) {
      m_file_list_structured_grid_type = DATA_PROVIDER_FILE_TYPE_EXT::NO_EXTENSION;

      std::string str_file = m_path_list_structured_grid + m_file_list_structured_grid;

      std::ifstream f_open_file(str_file);
      if (!f_open_file.is_open()) {
        std::cout << "Error: Unable to read structured grid file list." << std::endl;
        exit(EXIT_FAILURE);
      }

      while (!f_open_file.eof()) {
        std::string line;
        std::getline(f_open_file, line);

        int start_path = line.find_first_of("<") + 1;
        int end_path = line.find_first_of(">");

        int start_name = line.find_last_of("<") + 1;
        int end_name = line.find_last_of(">");

        std::string model_name = line.substr(start_name, end_name - start_name);

        std::cout << ". " << model_name << std::endl;

        m_uiname_list_structured_grid.push_back(model_name);
      }
      f_open_file.close();
    }
    else {
      exit(EXIT_FAILURE);
    }
    return true;

    return true;
    /*
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
    */
  }

  int DataProvider::GetNumberOfTransferFunctions ()
  {
    return (int)m_uiname_list_transfer_function.size();
  }

  std::vector<std::string>& DataProvider::GetTransferFunctionNameList ()
  {
    return m_uiname_list_transfer_function;
  }

  vis::TransferFunction* DataProvider::LoadTransferFunction (unsigned int id)
  {
    return nullptr;
  }

  std::string DataProvider::GetExtension (std::string filename)
  {
    int found = filename.find_last_of('.');
    std::string extension = filename.substr(size_t(found + 1));
    return extension;
  }

  int DataProvider::Split (std::vector<std::string>& tags, std::string line, std::string separator)
  {
    tags.clear();

    std::string line_str = line;
    while (line_str.find(separator) != std::string::npos) {
      tags.push_back(line_str.substr(0, line_str.find(separator)));
      line_str = line_str.substr(line_str.find(separator) + 1);
    }
    tags.push_back(line_str);

    return tags.size();
  }

  bool DataProvider::GetCSVStructuredGridTags (std::vector<std::string>& tags, unsigned int id)
  {
    std::string str_file = m_path_list_structured_grid + m_file_list_structured_grid;

    std::ifstream f_open_file(str_file);
    if (!f_open_file.is_open()) {
      std::cout << "Error: Unable to read structured grid file list." << std::endl;
      exit(EXIT_FAILURE);
    }

    std::string line;
    std::getline(f_open_file, line);
    if (line.substr(0, 3).compare("sep") == 0)
      std::getline(f_open_file, line);

    int grid_id = 0;
    while (!f_open_file.eof()) {
      std::getline(f_open_file, line);

      if (grid_id == id) {
        Split(tags, line, m_csv_str_grid->separator);
        return true;
      }
    }
    f_open_file.close();
    return false;
  }

  int DataProvider::GetBytesPerValueByType (std::string value_type)
  {
    if (value_type.compare("uchar") == 0 || value_type.compare("uint8") == 0) {
      return 1;
    }
    else if (value_type.compare("ushort") == 0 || value_type.compare("uint16") == 0) {
      return 2;
    }
  }

  std::pair<bool, std::string> DataProvider::CSVstructuredgrid::GetTagValue (std::vector<std::string>& values, std::string tag)
  {
    if (values.size() != tags.size()) return { false, " " };

    for (int i = 0; i < tags.size(); i++) {
      if (tags[i].compare(tag) == 0) {
        return { true, values[i] };
      }
    }
  }
}