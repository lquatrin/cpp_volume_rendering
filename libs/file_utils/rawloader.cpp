#include "rawloader.h"

#include <cerrno>

IRAWLoader::IRAWLoader (std::string filename, size_t bytes_per_pixel, size_t num_voxels, size_t type_size)
{
  m_data = NULL;
  m_filename = filename;
  m_bytesperpixel = bytes_per_pixel;
  m_numvoxels = num_voxels;
  m_typesize = type_size;

  FILE *fp;
  errno_t err;

  if((err = fopen_s(&fp, filename.c_str(), "rb")) != 0)
  {
    std::cout << "IRAWLoader: opening .raw file failed" << std::endl;
    exit(EXIT_FAILURE);
  }
  else
  {
    std::cout << "IRAWLoader: open .raw file successed" << std::endl;
  }

  m_data = (void*)malloc (m_numvoxels * type_size * sizeof(unsigned char));

  size_t tmp = fread(m_data, m_bytesperpixel, m_numvoxels, fp);
  if(tmp != m_numvoxels)
  {
    std::cout << "IRAWLoader: read .raw file failed. " << tmp << " bytes read != " << m_numvoxels << " bytes expected." << std::endl;
    fclose(fp);
    free(m_data);
    m_data = NULL;
    exit(EXIT_FAILURE);
  }
  else
  {
    std::cout << "IRAWLoader: read .raw file successed" << std::endl;
    fclose(fp);
  }
}

IRAWLoader::~IRAWLoader ()
{
  unsigned char* o_m_data = static_cast<unsigned char*>(m_data);
  free(o_m_data);
  m_data = NULL;
}

void* IRAWLoader::GetData ()
{
  return m_data;
}

bool IRAWLoader::IsLoaded ()
{
  return (m_data != NULL);
}