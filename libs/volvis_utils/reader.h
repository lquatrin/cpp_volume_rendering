/**
 * Classes to read Volumes and Transfer Functions
 * - VolumeReader:
 *  .pvm
 *  .raw
 *
 * - TransferFunctionReader:
 *  .tf1d
 *
 * Leonardo Quatrin Campagnolo
 * . campagnolo.lq@gmail.com
**/
#ifndef VOL_VIS_UTILS_VOLUME_AND_TF_READER_H
#define VOL_VIS_UTILS_VOLUME_AND_TF_READER_H

#include <volvis_utils/structuredgridvolume.h>
#include <volvis_utils/unstructuredgridvolume.h>
#include <volvis_utils/transferfunction.h>

#include <iostream>

namespace vis
{
  class VolumeReader
  {
  public:
    VolumeReader ();
    ~VolumeReader ();

    StructuredGridVolume* ReadStructuredVolume (std::string filepath);
  
  protected:
    StructuredGridVolume* readpvm (std::string filename);
    StructuredGridVolume* readpvmold (std::string filename);
    StructuredGridVolume* readraw (std::string filepath);
    StructuredGridVolume* readsyn (std::string filepath);

    UnstructuredGridVolume* readunsvol (std::string filepath);

  private:

  };

  class TransferFunctionReader
  {
  public:
    TransferFunctionReader ();
    ~TransferFunctionReader ();

    vis::TransferFunction* ReadTransferFunction (std::string file);
  
  protected:
    vis::TransferFunction* readtf1d (std::string file);

  private:

  };
}

#endif