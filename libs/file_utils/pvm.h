// Modified Version of .pvm file reader with DDS compression
// Code adapted from: https://github.com/dc2/volrenderer/blob/master/Formats

#ifndef FILE_UTILS_PVM_READER_H
#define FILE_UTILS_PVM_READER_H

#include <stdio.h>
#include <stdlib.h>

/* From viewer/mini/readme.html of V^3 package:
APPENDIX (B) PVM VOLUME FORMAT DESCRIPTION
  Similar to the PNM image format, the PVM volume format defines 
    volumetric data in an easily readable fashion:
      <MAGIC>\n
      <WIDTH> <HEIGHT> <DEPTH>\n
      <COMPONENTS>\n
      ...DATA...
  with
      <MAGIC>      = PVM ::: magic identifier
      <WIDTH>      = %d  ::: width of volume
      <HEIGHT>     = %d  ::: height of volume
      <COMPONENTS> = %d  ::: number of components
  
  For 8 bit data the number of components is 1, for 16 bit data 2 
    and for RGB movies it is 3. The voxel spacing is assumed to be uniform.
  
  A PVM example header for a 256x256x256 volume with 1 byte per voxel:
      PVM
      256 256 256
      1
      raw byte data...
  
  If the magic identifier is PVM2 or PVM3, this indicates newer versions of 
    the format, which include the voxel spacing in the header:
      <MAGIC>\n
      <WIDTH> <HEIGHT> <DEPTH>\n
      <VOXEL_WIDTH> <VOXEL_HEIGHT> <VOXEL_DEPTH>\n
      <COMPONENTS>\n
      ...DATA...
  
  A PVM3 example header for a 256x256x256 volume with a non-uniform voxel 
    spacing (the voxel spacing is 150% in z-direction) :
      PVM3
      256 256 256
      1 1 1.5
      1
      raw byte data...
  
  PVM files often come in a DDS compressed data format. In that case the data startes with the prefix "DDS v3d". You can uncompress those files with the dds tool.

  PVM
  The header is 3 ascii text lines consisting of "PVM" on the 
    first line, the width, height, and depth on the second line,
    and the number of components (bytes) per voxel on the third line.
    Since this is ascii the nature of the file can be readily determined
    with the UNIX command "head -3 thefile.pvm". The data then follows as binary data.

  Notes:
  The number of bytes in the binary data will be width*height*depth*components.

  The three lines of ascii header as a single string will be 
    null '\0' terminated, so one extra byte after the last '\n'.

  The voxels are assumed to be cubic.

  PVM2
  This is the same as PVM except there is an extra ascii line between the dimensions 
    and the number of components. This line has three numbers indicating the relative 
    size of the voxels, that is, providing support for non-square voxels.

  PMV3
  This is the same as PVM2 except that after the 4th line of the PVM2 header and 
    before the raw data there are 4 null terminated strings. These can of course 
    be used for anything but they are notionally intended for "description", 
    "courtesy", "parameters", and "comment". The maximum allowed length for each 
    of these is 256 bytes.
*/


#include <iostream>
#include <cstdint>

#include <cmath>

// TODO: ByteOrder
// Get the scale of the volume
class Pvm
{
public:
 //http://paulbourke.net/dataformats/reading/
 /*
   There are various datatypes which may be read, the simplest 
     is characters where no byte swapping is required. The next
     simplest is an unsigned short integer represented by 2 bytes.
     If the two bytes are read sequentially then the integer value
     on a big endian machine is 256*byte1+byte2. If the integer was
     written with a little endian machine such as a DOS/WINDOWS 
     computer then the integer is 256*byte2+byte1.
 */

public:
  Pvm (const char *file_name);
  ~Pvm ();

  void* GetData ();

  void GetDimensions (unsigned int* width, unsigned int* height, unsigned int* depth);
  void GetScale (double* sx, double* sy, double* sz);
  void GetScale (float* sx, float* sy, float* sz);
  int GetComponents ();


  float* GenerateNormalizeData ();
  //float* GenerateReescaledMinMaxData (bool normalized = false,
  //                                    float* fmin = NULL,
  //                                    float* fmax = NULL);
  //
  
  
  //void TransformData (unsigned int dst_bytes_per_voxel);

protected:
  void* pvm_data;

  unsigned int width, height, depth;
  float scalex, scaley, scalez;
  // 1 - uchar, 2 - ushort...
  unsigned int components;

private:
  // TODO: big endian and little endian. Only Big endian right now.
  // TODO: Convertion between data size? 8 -> 16 bits and 16 -> 8 bits.
  void* PostProcessData (unsigned char* data);
};

/////////////////////////////////////////////////////////////////////////////////
// PVM/DDS reader (8/16 bits):

// Copyright:
// (c) by Stefan Roettger, licensed under GPL 2+
// The Volume Library: http://www9.informatik.uni-erlangen.de/External/vollib/
// V^3 Package code: https://code.google.com/p/vvv/

#define DDS_ISINTEL (*((unsigned char *)(&DDS_INTEL) + 1) == 0)

#define ERRORMSG() DDSV3::errormsg(__FILE__,__LINE__)
#define MEMERROR() DDSV3::errormsg(__FILE__,__LINE__)
#define IOERROR() DDSV3::errormsg(__FILE__,__LINE__)



class DDSV3
{
public:
  unsigned char* readPVMvolume (const char* filename,
                                unsigned int* width,
                                unsigned int* height,
                                unsigned int* depth,
                                unsigned int* components = NULL,
                                float* scalex = NULL,
                                float* scaley = NULL,
                                float* scalez = NULL,
                                unsigned char** description = NULL,
                                unsigned char** courtesy = NULL,
                                unsigned char** parameter = NULL,
                                unsigned char** comment = NULL);

  unsigned char* readPNMimage (const char *filename,
                               unsigned int *width,
                               unsigned int *height,
                               unsigned int *components);

  // helper functions for DDS:
  static inline void errormsg (const char *file, int line)
  {
    fprintf(stderr, " fatal error in <%s> at line %d!\n", file, line);
    
    //if (fatal == VR_ERROR_NONFATAL) fprintf(stderr, "warning");
    //else if (fatal == VR_ERROR_MEM) fprintf(stderr, "insufficient memory");
    //else if (fatal == VR_ERROR_IO) fprintf(stderr, "io error");
    //else if (fatal == VR_ERROR_CODE) fprintf(stderr, "unimplemented code");
    //else fprintf(stderr, "fatal error");
    //if (msg != NULL) fprintf(stderr, "description: %s\n", msg);

    exit(1);
  }

  static inline unsigned int DDS_shiftl (const unsigned int value, const unsigned int bits)
  {
    return((bits >= 32) ? 0 : value << bits);
  }

  static inline unsigned int DDS_shiftr (const unsigned int value, const unsigned int bits)
  {
    return((bits >= 32) ? 0 : value >> bits);
  }

  static inline void DDS_swapuint (unsigned int *x)
  {
    unsigned int tmp = *x;

    *x = ((tmp & 0xff) << 24) |
      ((tmp & 0xff00) << 8) |
      ((tmp & 0xff0000) >> 8) |
      ((tmp & 0xff000000) >> 24);
  }

  static inline int DDS_code (int bits)
  {
    return(bits > 1 ? bits - 1 : bits);
  }

  static inline int DDS_decode (int bits)
  {
    return(bits >= 1 ? bits + 1 : bits);
  }

  static void swapshort (unsigned char* ptr, unsigned int size)
  {
    unsigned int i;

    unsigned char lo, hi;

    for (i = 0; i < size; i++)
    {
      lo = ptr[0];
      hi = ptr[1];
      *ptr++ = hi;
      *ptr++ = lo;
    }
  }

public:
  void DDS_initbuffer ()
  {
    DDS_buffer = 0;
    DDS_bufsize = 0;
  }

  inline void DDS_clearbits ()
  {
    DDS_cache = NULL;
    DDS_cachepos = 0;
    DDS_cachesize = 0;
  }

  inline unsigned int DDS_readbits (unsigned int bits)
  {
    unsigned int value;

    if (bits < DDS_bufsize)
    {
      DDS_bufsize -= bits;
      value = DDSV3::DDS_shiftr(DDS_buffer, DDS_bufsize);
    }
    else
    {
      value = DDSV3::DDS_shiftl(DDS_buffer, bits - DDS_bufsize);

      if (DDS_cachepos >= DDS_cachesize) DDS_buffer = 0;
      else
      {
        DDS_buffer = *((unsigned int *)&DDS_cache[DDS_cachepos]);
        if (DDS_ISINTEL) DDSV3::DDS_swapuint(&DDS_buffer);
        DDS_cachepos += 4;
      }

      DDS_bufsize += 32 - bits;
      value |= DDSV3::DDS_shiftr(DDS_buffer, DDS_bufsize);
    }

    DDS_buffer &= DDSV3::DDS_shiftl(1, DDS_bufsize) - 1;

    return(value);
  }

  inline void DDS_loadbits (unsigned char* data, unsigned int size)
  {
    DDS_cache = data;
    DDS_cachesize = size;

    if ((DDS_cache = (unsigned char*)realloc(DDS_cache, DDS_cachesize + 4)) == NULL) MEMERROR();
    *((unsigned int *)&DDS_cache[DDS_cachesize]) = 0;

    DDS_cachesize = 4 * ((DDS_cachesize + 3) / 4);
    if ((DDS_cache = (unsigned char*)realloc(DDS_cache, DDS_cachesize)) == NULL) ERRORMSG();
  }


public:
  void DDS_decode (unsigned char* chunk, unsigned int size,
                   unsigned char** data, unsigned int* bytes,
                   unsigned int block = 0);

  void DDS_interleave (unsigned char* data,
                       unsigned int bytes,
                       unsigned int skip,
                       unsigned int block = 0);

  void DDS_deinterleave (unsigned char* data,
                         unsigned int bytes,
                         unsigned int skip,
                         unsigned int block = 0,
                         bool restore = false);
  
  unsigned char* readDDSfile (const char *filename, unsigned int *bytes);

  unsigned char* readRAWfiled (FILE *file, unsigned int *bytes);
  unsigned char* readRAWfile (const char *filename, unsigned int *bytes);


  unsigned char* DDS_cache;
  unsigned int DDS_cachepos, DDS_cachesize;

  unsigned int DDS_buffer;
  unsigned int DDS_bufsize;

  unsigned short int DDS_INTEL = 1;
protected:
  const char* DDS_ID = "DDS v3d\n";
  const char* DDS_ID2 = "DDS v3e\n";

private:

  /*void writeDDSfile(const char *filename,unsigned char *data,unsigned int bytes,unsigned int skip=0,unsigned int strip=0,BOOLINT nofree=FALSE);

  void writeRAWfile(const char *filename,unsigned char *data,unsigned int bytes,BOOLINT nofree=FALSE);

  void writePNMimage(const char *filename,unsigned char *image,unsigned int width,unsigned int height,unsigned int components,BOOLINT dds=FALSE);

  void writePVMvolume(const char *filename,unsigned char *volume,
  unsigned int width,unsigned int height,unsigned int depth,unsigned int components=1,
  float scalex=1.0f,float scaley=1.0f,float scalez=1.0f,
  unsigned char *description=NULL,
  unsigned char *courtesy=NULL,
  unsigned char *parameter=NULL,
  unsigned char *comment=NULL);

  int checkfile(const char *filename);
  unsigned int checksum(unsigned char *data,unsigned int bytes);

  void swapbytes(unsigned char *data,long long bytes);
  void convbytes(unsigned char *data,long long bytes);
  void convfloat(unsigned char **data,long long bytes);
  void convrgb(unsigned char **data,long long bytes);

  unsigned char *quantize(unsigned char *volume,
  long long width,long long height,long long depth,
  BOOLINT msb=TRUE,
  BOOLINT linear=FALSE,BOOLINT nofree=FALSE);

  char *processPVMvolume(const char *filename);*/
};
#endif