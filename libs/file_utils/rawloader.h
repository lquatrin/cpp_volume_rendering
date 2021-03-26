#ifndef FILE_UTILS_RAWLOADER_H
#define FILE_UTILS_RAWLOADER_H

#include <cstring>
#include <iostream>
#include <stdexcept>

class IRAWLoader
{
public:
  IRAWLoader (std::string fileName, size_t bytes_per_pixel, size_t num_voxels, size_t type_size);
  ~IRAWLoader ();

  void* GetData ();
  bool IsLoaded ();
private:
  std::string m_filename;
  size_t m_bytesperpixel;
  size_t m_numvoxels;
  size_t m_typesize;
  void* m_data;
};

#endif