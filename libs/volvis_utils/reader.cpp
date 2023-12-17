/**
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#include "reader.h"

#include <file_utils/pvm.h>
#include <file_utils/pvm_old.h>
#include <file_utils/rawloader.h>

#include <fstream>
#include <array>

#include <volvis_utils/transferfunction1d.h>

namespace vis
{
  VolumeReader::VolumeReader ()
  {

  }

  VolumeReader::~VolumeReader ()
  {

  }

  StructuredGridVolume* VolumeReader::ReadStructuredVolume (std::string filepath)
  {
    StructuredGridVolume* ret = nullptr;

    int found = filepath.find_last_of('.');
    std::string extension = filepath.substr(size_t(found + 1));

    printf(". Reading Structured Grid Volume... ");
    if (extension.compare("pvm") == 0) {
      ret = readpvm(filepath);
    }
    else if (extension.compare("pvmold") == 0) {
      ret = readpvmold(filepath);
    }
    else if (extension.compare("raw") == 0) {
      ret = readraw(filepath);
    }
    else if (extension.compare("syn") == 0) {
      ret = readsyn(filepath);
    }
    else if (extension.compare("nrrd") == 0) {
      ret = readnrrd(filepath);
    }
    else if (extension.compare("nhrd") == 0) {
      ret = readnhrd(filepath);
    }
    else if (extension.compare("dat") == 0) {
      ret = readdat(filepath);
    }
    printf("DONE\n");

    return ret;
  }

  void VolumeReader::SetArrayDataFromRawFile (std::string filepath, StructuredGridVolume* sg, int bytes_per_value)
  {
    int fw = sg->GetWidth(), fh = sg->GetHeight(), fd = sg->GetDepth();

    IRAWLoader rawLoader = IRAWLoader(filepath, bytes_per_value, fw * fh * fd, bytes_per_value);

    vis::DataStorageSize data_tp;
    
    void* scalar_values = nullptr;
    // GLushort - 16 bits - converting to float
    if (bytes_per_value == sizeof(unsigned short))
    {
      data_tp = vis::DataStorageSize::_16_BITS;
      scalar_values = new unsigned short[fw * fh * fd];

      unsigned short* us_scalar_values = static_cast<unsigned short*>(scalar_values);
      unsigned short* b = static_cast<unsigned short*>(rawLoader.GetData());

      for (int i = 0; i < fw * fh * fd; i++)
        us_scalar_values[i] = b[i];
    }
    // GLubyte - 8 bits - converting to float
    else if (bytes_per_value == sizeof(unsigned char))
    {
      data_tp = vis::DataStorageSize::_8_BITS;
      scalar_values = new unsigned char[fw * fh * fd];

      unsigned char* uc_scalar_values = static_cast<unsigned char*>(scalar_values);
      unsigned char* b = static_cast<unsigned char*>(rawLoader.GetData());

      for (int i = 0; i < fw * fh * fd; i++)
        uc_scalar_values[i] = b[i];
    }

    // We won't delete the scalar_values, because it will be stored at structured grid volume...
    sg->SetArrayData(scalar_values, data_tp);
  }

  StructuredGridVolume* VolumeReader::readpvm (std::string filename)
  {
    StructuredGridVolume* ret = nullptr;

    printf("Started  -> Read Volume From .pvm File\n");
    printf("  - File .pvm Path: %s\n", filename.c_str());

    unsigned int width, height, depth, components;
    double scalex, scaley, scalez;

    Pvm fpvm(filename.c_str());

    fpvm.GetDimensions(&width, &height, &depth);
    components = fpvm.GetComponents();
    fpvm.GetScale(&scalex, &scaley, &scalez);

    assert(components > 0);

    vis::DataStorageSize data_tp;
    void* scalar_values = nullptr;

    // GLubyte - 8 bits
    if (components == 1)
    {
      data_tp = vis::DataStorageSize::_8_BITS;
    
      scalar_values = new unsigned char[width * height * depth];
      unsigned char* u_sv = static_cast<unsigned char*>(scalar_values);

      unsigned char* vol_data = static_cast<unsigned char*>(fpvm.GetData());
      for (int i = 0; i < width * height * depth; i++)
        u_sv[i] = (unsigned char)vol_data[i];
    }
    // GLushort - 16 bits
    else if (components == 2)
    {
      data_tp = vis::DataStorageSize::_16_BITS;
      
      scalar_values = new unsigned short[width * height * depth];
      unsigned short* u_sv = static_cast<unsigned short*>(scalar_values);

      unsigned short* vol_data = static_cast<unsigned short*>(fpvm.GetData());
      for (int i = 0; i < width * height * depth; i++)
        u_sv[i] = (unsigned short)vol_data[i];
    }

    ret = new StructuredGridVolume(filename, width, height, depth);
    ret->SetScale(scalex, scaley, scalez);
    ret->SetName(filename);

    // We won't delete the vol_data, because it will be stored at 
    //   structured grid volume...
    ret->SetArrayData(scalar_values, data_tp);

    printf("  - Volume Name     : %s\n", filename.c_str());
    printf("  - Volume Size     : [%d, %d, %d]\n", width, height, depth);

    printf("Finished -> Read Volume From .pvm File\n");

    return ret;
  }

  StructuredGridVolume* VolumeReader::readraw (std::string filepath)
  {
    StructuredGridVolume* sg_ret = nullptr;

    printf("Started  -> Read Volume From .raw File\n");
    printf("  - File .raw Path: %s\n", filepath.c_str());

    std::ifstream iffile(filepath.c_str());
    if (iffile.is_open())
    {
      int foundinit = filepath.find_last_of('\\');
      std::string filename = filepath.substr(foundinit + 1);
      printf("  - File .raw: %s\n", filename.c_str());

      int foundfp = filename.find_last_of('.');
      filename = filename.substr(0, foundfp);

      int foundsizes = filename.find_last_of('.');
      std::string t_filesizes = filename.substr(foundsizes + 1, filename.size() - foundsizes);

      filename = filename.substr(0, filename.find_last_of('.'));

      int foundbytesize = filename.find_last_of('.');
      std::string t_filebytesize = filename.substr(foundbytesize + 1, filename.size() - foundbytesize);

      int fw, fh, fd;
      int bytes_per_value;

      // Read the Volume Sizes
      int foundd = t_filesizes.find_last_of('x');
      fd = atoi(t_filesizes.substr(foundd + 1, t_filesizes.size() - foundd).c_str());

      t_filesizes = t_filesizes.substr(0, t_filesizes.find_last_of('x'));

      int foundh = t_filesizes.find_last_of('x');
      fh = atoi(t_filesizes.substr(foundh + 1, t_filesizes.size() - foundh).c_str());

      t_filesizes = t_filesizes.substr(0, t_filesizes.find_last_of('x'));

      int foundw = t_filesizes.find_last_of('x');
      fw = atoi(t_filesizes.substr(foundw + 1, t_filesizes.size() - foundw).c_str());

      // Byte Size
      bytes_per_value = atoi(t_filebytesize.c_str());

      sg_ret = new StructuredGridVolume(filename, fw, fh, fd);
      sg_ret->SetScale(1.0, 1.0, 1.0);
      sg_ret->SetName(filepath);

      SetArrayDataFromRawFile(filepath, sg_ret, bytes_per_value);

      printf("  - Volume Name     : %s\n", filepath.c_str());
      printf("  - Volume Size     : [%d, %d, %d]\n", fw, fh, fd);
      printf("  - Volume Byte Size: %d\n", bytes_per_value);

      iffile.close();
      printf("Finished -> Read Volume From .raw File\n");
    }
    else {
      printf("Finished -> Error on opening .raw file\n");
    }

    return sg_ret;
  }

  StructuredGridVolume* VolumeReader::readpvmold (std::string filename)
  {
    StructuredGridVolume* ret = nullptr;
  
    printf("Started  -> Read Volume From .pvmold File\n");
    printf("  - File .pvmold Path: %s\n", filename.c_str());
  
    unsigned int width, height, depth, components;
    double scalex, scaley, scalez;

    PvmOld fpvm(filename.c_str());
    fpvm.GetDimensions(&width, &height, &depth);
    components = fpvm.GetComponents();
    fpvm.GetScale(&scalex, &scaley, &scalez);

    // Get Volume Data
    float* vol_data = 
      //fpvm.GetData()
      fpvm.GenerateReescaledMinMaxData()
    ;

    double max_density = 1.0;
    if (components == 1)
      max_density = (256.0 - 1.0);
    else if (components == 2)
      max_density = (65536.0 - 1.0);

    double* aux_output_data = new double[width * height * depth];
    for (int i = 0; i < width * height * depth; i++)
      aux_output_data[i] = (double)vol_data[i];

    GLfloat* scalar_values = new GLfloat[width * height * depth];
    for (int i = 0; i < width * height * depth; i++)
      scalar_values[i] = ((GLfloat)((int)aux_output_data[i]) / max_density);

    assert(components > 0);

    delete[] vol_data;
    delete[] aux_output_data;

    ret = new StructuredGridVolume(filename, width, height, depth);
    ret->SetScale(scalex, scaley, scalez);
    ret->SetName(filename);

    // We won't delete the vol_data, because it will be stored at 
    //   structured grid volume...
    ret->SetArrayData(scalar_values, vis::DataStorageSize::_NORMALIZED_F);

    printf("  - Volume Name     : %s\n", filename.c_str());
    printf("  - Volume Size     : [%d, %d, %d]\n", width, height, depth);

    printf("Finished -> Read Volume From .pvmold File\n");
    
    return ret;
  }

  StructuredGridVolume* VolumeReader::readsyn (std::string filepath)
  {
    StructuredGridVolume* sg_ret = nullptr;

    printf("Started  -> Read Volume From .syn File\n");
    printf("  - File .syn Path: %s\n", filepath.c_str());

    std::ifstream iffile(filepath.c_str());
    if (iffile.is_open())
    {
      int width, height, depth;
      iffile >> width >> height >> depth;
      std::cout << "  - Volume Size: [" << width << ", " << height << ", " << depth << "]" << std::endl;
  
      sg_ret = new StructuredGridVolume(filepath, width, height, depth);
      sg_ret->SetScale(1.0, 1.0, 1.0);
      sg_ret->SetName(filepath);
      
      unsigned char* syn_data = new unsigned char[width*height*depth];

      
      int new_data = 0;
      while (iffile >> new_data)
      {
        if (new_data == 1)
        {
          int x0, y0, z0, x1, y1, z1, v;
          iffile >> x0 >> y0 >> z0 >> x1 >> y1 >> z1 >> v;
          for (int x = x0; x < x1; x++)
          {
            for (int y = y0; y < y1; y++)
            {
              for (int z = z0; z < z1; z++)
              {
                syn_data[x + (width * y) + (width * height * z)] = unsigned char(v);
              }
            }
          }
        }
        //else if (new_data == 2)
        //{
        //  int x0, y0, z0, x1, y1, z1, v, d;
        //  iffile >> x0 >> y0 >> z0 >> x1 >> y1 >> z1 >> v >> d;
        //  for (int x = x0; x < x1; x++)
        //  {
        //    for (int y = y0; y < y1; y++)
        //    {
        //      for (int z = z0; z < z1; z++)
        //      {
        //        glm::vec3 dv(x1 - x0, y1 - y0, z1 - z0);
        //        dv = glm::normalize(dv);
        //
        //        glm::vec3 ap = glm::vec3(x, y, z) - glm::vec3(x0, y0, z0);
        //
        //        glm::vec3 apdv = glm::vec3(ap.y*dv.z - ap.z*dv.y,
        //                                   ap.z*dv.x - ap.x*dv.z,
        //                                   ap.x*dv.y - ap.y*dv.x);
        //
        //        float dist = glm::sqrt(apdv.x * apdv.x + apdv.y * apdv.y + apdv.z * apdv.z);
        //        if (dist < d)
        //          sync_data[x + (width)*y + (width * height) * z] = GLubyte(v);
        //      }
        //    }
        //  }
        //}
        else
        {
          int xt, yt, zt, v;
          iffile >> xt >> yt >> zt >> v;
          syn_data[xt + (width * yt) + (width * height * zt)] = unsigned char(v);
        }
      }
      
      // We won't delete the scalar_values, because it will be stored at 
      //   structured grid volume...
      sg_ret->SetArrayData(syn_data, vis::DataStorageSize::_8_BITS);

      printf("  - Volume Name     : %s\n", filepath.c_str());
      printf("  - Volume Size     : [%d, %d, %d]\n", width, height, depth);

      iffile.close();
      printf("Finished -> Read Volume From .syn File\n");
    }
    else {
      printf("Finished -> Error on opening .syn file\n");
    }

    return sg_ret;
  }

  StructuredGridVolume* VolumeReader::readnrrd (std::string filepath)
  {
    StructuredGridVolume* sg_ret = nullptr;

    printf("Started  -> Read Volume From .nrrd File\n");
    printf("  - File .nrrd Path: %s\n", filepath.c_str());

    std::string path = filepath;
    std::string name = filepath;
    int t1 = filepath.find_last_of('\\');
    int t2 = filepath.find_last_of('/');
    int t = glm::max(t1, t2);
    if (t > -1) {
      path = filepath.substr(0, t);
      name = filepath.substr(t + 1);
    }

    std::ifstream iffile(filepath.c_str());
    if (iffile.is_open()) {
      std::string version;
      std::getline(iffile, version);

      std::string type;
      std::string dimension;
      std::string space;
      glm::ivec3 resolution;
      std::array<glm::vec3, 3> space_directions;
      glm::vec3 slicethickness(1, 1, 1);
      std::string kinds;
      std::string endian;
      std::string encoding;
      std::string space_origin;
      std::string data_file;

      // read keys
      while (!iffile.eof()) {
        std::string line;
        std::getline(iffile, line);
      
        std::cout << line << std::endl;
        
        if (line.find_first_of('#') == 0) continue;

        std::string key = line.substr(0, line.find_first_of(':'));
        std::string content = line.substr(line.find_first_of(":") + 1);

        // skip ' ' and '\t'
        int count_skip = 0;
        while (content[count_skip] == ' ' || content[count_skip] == '\t') {
          count_skip += 1;
        }
        content = content.substr(count_skip);

        if (key.find("type") == 0) {
          type = content;
        }
        else if (key.find("dimension") == 0) {
          dimension = content;
        }
        else if (key.find("space") == 0) {
          space = content;
        }
        else if (key.find("sizes") == 0) {
          std::string str_resolution = content;

          std::vector<int> vec_resolution;
          while (str_resolution.find(' ') != std::string::npos) {
            std::string size_l = str_resolution.substr(0, str_resolution.find(' '));
            vec_resolution.push_back(std::atoi(size_l.c_str()));
            str_resolution = str_resolution.substr(str_resolution.find(' ') + 1);
          }
          vec_resolution.push_back(std::atoi(str_resolution.c_str()));

          resolution = glm::ivec3(vec_resolution[0], vec_resolution[1], vec_resolution[2]);
        }
        else if (key.find("space directions") == 0) {
          space_directions[0] = glm::vec3(1, 0, 0);
          space_directions[1] = glm::vec3(0, 1, 0);
          space_directions[2] = glm::vec3(0, 0, 1);
        }
        else if (key.find("kinds") == 0) {
          kinds = content;
        }
        else if (key.find("endian") == 0) {
          endian = content;
        }
        else if (key.find("encoding") == 0) {
          encoding = content;
        }
        else if (key.find("space origin") == 0) {
          space_origin = content;
        }
        else if (key.find("data file") == 0) {
          data_file = content;
        }
      }
      iffile.close();

      sg_ret = new StructuredGridVolume(name, resolution.x, resolution.y, resolution.z);
      sg_ret->SetScale(slicethickness.x, slicethickness.y, slicethickness.z);

      std::string volume_data_array_file = path + "/" + data_file;

      int bytes_per_value = 1;
      if ((int)type.find("uint8") > -1) {
        bytes_per_value = sizeof(unsigned char);
      }
      if ((int)type.find("uint16") > -1) {
        bytes_per_value = sizeof(unsigned short);
      }

      SetArrayDataFromRawFile(volume_data_array_file, sg_ret, bytes_per_value);
    }
    else {
      printf("Finished -> Error on opening .nrrd file\n");
    }

    return sg_ret;
  }

  StructuredGridVolume* VolumeReader::readnhrd (std::string filepath)
  {
    return readnrrd(filepath);
  }

  StructuredGridVolume* VolumeReader::readdat (std::string filepath)
  {
    StructuredGridVolume* sg_ret = nullptr;

    printf("Started  -> Read Volume From .dat File\n");
    printf("  - File .dat Path: %s\n", filepath.c_str());

    std::ifstream iffile(filepath.c_str());
    if (iffile.is_open()) {
      std::string path = filepath;
      std::string name = filepath;
      int t1 = filepath.find_last_of('\\');
      int t2 = filepath.find_last_of('/');
      int t = glm::max(t1, t2);
      if (t > -1) {
        path = filepath.substr(0, t);
        name = filepath.substr(t+1);
      }
      
      std::string objectfilename;
      std::string taggedfilename;
      glm::ivec3 resolution;
      glm::vec3 slicethickness(1.0,1.0,1.0);
      std::string format;
      int nbrtags;
      std::string objecttype;
      std::string objectmodel;
      std::string gridtype;
      std::string modality;
      int timestep;
      
      // read keys
      while (!iffile.eof()) {
        std::string line;
        std::getline(iffile, line);
      
        std::cout << line << std::endl;
        
        std::string key = line.substr(0, line.find_first_of(':'));
        std::string content = line.substr(line.find_first_of(":") + 1);

        // skip ' ' and '\t'
        int count_skip = 0;
        while (content[count_skip] == ' ' || content[count_skip] == '\t') {
          count_skip += 1;
        }
        content = content.substr(count_skip);

        if (key.find("ObjectFileName") == 0) {
          objectfilename = content;
        }
        else if (key.find("TaggedFileName") == 0) {
          taggedfilename = content;
        }
        else if (key.find("Resolution") == 0) {
          std::string str_resolution = content;
          
          std::vector<int> vec_resolution;
          while (str_resolution.find(' ') != std::string::npos) {
            std::string size_l = str_resolution.substr(0, str_resolution.find(' '));
            vec_resolution.push_back(std::atoi(size_l.c_str()));
            str_resolution = str_resolution.substr(str_resolution.find(' ') + 1);
          }
          vec_resolution.push_back(std::atoi(str_resolution.c_str()));

          resolution = glm::ivec3(vec_resolution[0], vec_resolution[1], vec_resolution[2]);
        }
        else if (key.find("SliceThickness") == 0) {
          std::string str_slicethickness = content;

          std::vector<float> vec_thickness;
          while (str_slicethickness.find(' ') != std::string::npos) {
            std::string size_l = str_slicethickness.substr(0, str_slicethickness.find(' '));
            vec_thickness.push_back(std::stof(size_l.c_str()));
            str_slicethickness = str_slicethickness.substr(str_slicethickness.find(' ') + 1);
          }
          vec_thickness.push_back(std::stof(str_slicethickness.c_str()));

          slicethickness = glm::vec3(vec_thickness[0], vec_thickness[1], vec_thickness[2]);
        }
        else if (key.find("Format") == 0) {
          format = content;
        }
        // TODO
        else if (key.find("NbrTags") == 0) {
          //int nbrtags;
        }
        // TODO
        else if (key.find("ObjectType") == 0) {
          //std::string objecttype;
        }
        // TODO
        else if (key.find("ObjectModel") == 0) {
         // std::string objectmodel;
        }
        // TODO
        else if (key.find("GridType") == 0) {
          //std::string gridtype;
        }
        // TODO
        else if (key.find("Modality") == 0) {
          //std::string modality;
        }
        // TODO
        else if (key.find("TimeStep") == 0) {
          //int timestep;
        }
      }
      iffile.close();

      sg_ret = new StructuredGridVolume(name, resolution.x, resolution.y, resolution.z);
      sg_ret->SetScale(slicethickness.x, slicethickness.y, slicethickness.z);
      
      std::string volume_data_array_file = path + "/" + objectfilename;

      int bytes_per_value = 1;
      if ((int)format.find("UCHAR") > -1) {       // 8 bits
        bytes_per_value = sizeof(unsigned char);
      }
      else if ((int)format.find("USHORT") > -1) { // 16 bits
        bytes_per_value = sizeof(unsigned short);
      }

      SetArrayDataFromRawFile(volume_data_array_file, sg_ret, bytes_per_value);
    }
    else {
      printf("Finished -> Error on opening .dat file\n");
    }

    return sg_ret;
  }

  UnstructuredGridVolume* VolumeReader::readunsvol (std::string filepath)
  {
    UnstructuredGridVolume* sg_ret = nullptr;

    printf("Started  -> Read Volume From .unsvol File\n");
    printf("  - File .unsvol Path: %s\n", filepath.c_str());

    std::ifstream iffile(filepath.c_str());
    if (iffile.is_open())
    {
      //int width, height, depth;
      //iffile >> width >> height >> depth;
      //std::cout << "  - Volume Size: [" << width << ", " << height << ", " << depth << "]" << std::endl;
      //
      //sg_ret = new StructuredGridVolume(filepath, width, height, depth);
      //sg_ret->SetScale(1.0, 1.0, 1.0);
      //sg_ret->SetName(filepath);
      //
      //unsigned char* syn_data = new unsigned char[width * height * depth];
      //
      //
      //int new_data = 0;
      //while (iffile >> new_data)
      //{
      //  if (new_data == 1)
      //  {
      //    int x0, y0, z0, x1, y1, z1, v;
      //    iffile >> x0 >> y0 >> z0 >> x1 >> y1 >> z1 >> v;
      //    for (int x = x0; x < x1; x++)
      //    {
      //      for (int y = y0; y < y1; y++)
      //      {
      //        for (int z = z0; z < z1; z++)
      //        {
      //          syn_data[x + (width * y) + (width * height * z)] = unsigned char(v);
      //        }
      //      }
      //    }
      //  }
      //  //else if (new_data == 2)
      //  //{
      //  //  int x0, y0, z0, x1, y1, z1, v, d;
      //  //  iffile >> x0 >> y0 >> z0 >> x1 >> y1 >> z1 >> v >> d;
      //  //  for (int x = x0; x < x1; x++)
      //  //  {
      //  //    for (int y = y0; y < y1; y++)
      //  //    {
      //  //      for (int z = z0; z < z1; z++)
      //  //      {
      //  //        glm::vec3 dv(x1 - x0, y1 - y0, z1 - z0);
      //  //        dv = glm::normalize(dv);
      //  //
      //  //        glm::vec3 ap = glm::vec3(x, y, z) - glm::vec3(x0, y0, z0);
      //  //
      //  //        glm::vec3 apdv = glm::vec3(ap.y*dv.z - ap.z*dv.y,
      //  //                                   ap.z*dv.x - ap.x*dv.z,
      //  //                                   ap.x*dv.y - ap.y*dv.x);
      //  //
      //  //        float dist = glm::sqrt(apdv.x * apdv.x + apdv.y * apdv.y + apdv.z * apdv.z);
      //  //        if (dist < d)
      //  //          sync_data[x + (width)*y + (width * height) * z] = GLubyte(v);
      //  //      }
      //  //    }
      //  //  }
      //  //}
      //  else
      //  {
      //    int xt, yt, zt, v;
      //    iffile >> xt >> yt >> zt >> v;
      //    syn_data[xt + (width * yt) + (width * height * zt)] = unsigned char(v);
      //  }
      //}
      //
      //// We won't delete the scalar_values, because it will be stored at 
      ////   structured grid volume...
      //sg_ret->SetArrayData(syn_data, vis::DataStorageSize::_8_BITS);


      iffile.close();
      printf("Finished -> Read Volume From .unsvol File\n");
    }
    else {
      printf("Finished -> Error on opening .unsvol file\n");
    }

    return sg_ret;
  }

  TransferFunctionReader::TransferFunctionReader ()
  {

  }

  TransferFunctionReader::~TransferFunctionReader ()
  {

  }
  
  vis::TransferFunction* TransferFunctionReader::ReadTransferFunction (std::string file)
  {
    TransferFunction* tf_ret = NULL;

    int found = file.find_last_of('.');
    std::string extension = file.substr(found + 1);

    printf(". Reading Transfer Function\n");
    printf(" - File: %s\n", file.c_str());

    if (extension.compare("tf1d") == 0)
      tf_ret = readtf1d(file);

    return tf_ret;
  }

  TransferFunction* TransferFunctionReader::readtf1d (std::string file)
  {
    std::ifstream myfile(file);
    if (myfile.is_open())
    {
      // we're always considering linear interpolation
      // TODO: also consider cubic interpolation
      std::string interpolation;
      std::getline(myfile, interpolation);
      printf(" - Interpolation: %s\n", interpolation.c_str());

      TransferFunction1D* ret_tf = NULL;
      int init;
      myfile >> init;

      if (init == 2)
      {
        int max_density;
        myfile >> max_density;

        int extuse;
        myfile >> extuse;

        ret_tf = new TransferFunction1D(max_density);
        ret_tf->SetExtinctionCoefficientInput(extuse == 1);
      }
      else if (init == 1)
      {
        int max_density;
        myfile >> max_density;

        ret_tf = new TransferFunction1D(max_density);
      }
      else
      {
        ret_tf = new TransferFunction1D();
      }

      int cpt_rgb_size;
      myfile >> cpt_rgb_size;
      double r, g, b, a;
      int isovalue;
      for (int i = 0; i < cpt_rgb_size; i++)
      {
        myfile >> r >> g >> b >> isovalue;
        ret_tf->AddRGBControlPoint(TransferControlPoint(r, g, b, isovalue));
      }

      int cpt_alpha_size;
      myfile >> cpt_alpha_size;
      for (int i = 0; i < cpt_alpha_size; i++)
      {
        myfile >> a >> isovalue;
        ret_tf->AddAlphaControlPoint(TransferControlPoint(a, isovalue));
      }

      myfile.close();

      //if (interpolation.compare ("linear") == 0)
      //  tf->m_interpolation_type = TFInterpolationType::LINEAR;
      //else if (interpolation.compare ("cubic") == 0)
      //  tf->m_interpolation_type = TFInterpolationType::CUBIC;

      int foundname = file.find_last_of('\\');
      std::string tfname = file.substr(foundname + 1);
      ret_tf->SetName(file);

      return ret_tf;
    }
    return nullptr;
  }
}