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

    void SetArrayDataFromRawFile (std::string filepath, StructuredGridVolume* sg, int bytes_per_value);

  protected:
    StructuredGridVolume* readpvm (std::string filename);
    StructuredGridVolume* readpvmold (std::string filename);
    StructuredGridVolume* readraw (std::string filepath);
    StructuredGridVolume* readsyn (std::string filepath);
    /**
     * http://teem.sourceforge.net/nrrd/format.html
     * -------------------------------------------
     * NRRD000X
     * <field>: <desc>
     * <field>: <desc>
     * # <comment>
     * ...
     * <field>: <desc>
     * <key>:=<value>
     * <key>:=<value>
     * <key>:=<value>
     * # <comment>
     * 
     * <data><data><data><data><data><data>...
     * -------------------------------------------
     * NRRD0001: (and NRRD00.01 for circa 1998 files) original and most basic version
     * NRRD0002: added key/value pairs
     * NRRD0003: added "kinds:" field,
     * NRRD0004: added "thicknesses:" and "sample units" fields, general space and orientation information ("space:", "space dimension:", "space directions:", "space origin:", and "space units:" fields) , and the ability for the "data file:" field to identify multiple data files.
     * NRRD0005: added "measurement frame:" field (should have been figured out for NRRD0004).
     */
    StructuredGridVolume* readnrrd (std::string filepath);
    // File structure from open scivis datasets, equal to nrrd
    StructuredGridVolume* readnhrd (std::string filepath);
    // File structure from zurich datasets
    StructuredGridVolume* readdat (std::string filepath);

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