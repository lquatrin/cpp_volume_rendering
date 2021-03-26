These file are parte of package that contains the V^3, the Versatile Volume Viewer.
Copyright (c) 2003-2016 by Stefan Roettger.

The Versatile Volume Viewer is a volume renderer that displays regular
volumetric data such as medical CT or MRI scans in 3D and at
real-time. For best rendering performance an NVIDIA GTX 560, 570 or
580 graphics card is recommended!

The volume viewer is licensed under the terms of the GPL (see
http://www.gnu.org/copyleft/ for more information on the license).
Any commercial use of the code or parts of it requires the explicit
permission of the author! The source code is distributed with
ABSOLUTELY NO WARRANTY; not even for MERCHANTABILITY etc.!

The author's contact address is:

   mailto:stefan@stereofx.org
   http://stereofx.org

In order to compile the viewer under Linux simply type "build.sh" in a
shell. If CMake is installed, type "cmake . && make". The viewer
requires OpenGL and the GLUT library as the only dependencies.

The installation of OpenGL and GLUT is vendor specific: On MacOS X it
is already installed with the XCode development package, on Linux it
comes with the "mesa" and "free-glut3-dev" package whereas on Windows
it is usually installed with the MSVC IDE.

Under Windows either use the CMake GUI to produce a Visual C++ project
or use cygwin in the following way:

1. Download cygwin from http://cygwin.com
2. Run the installer program and make sure to select "Devel"
3. After the installation finishes, download the V^3
4. Unzip the V^3 package
5. Double-click the cygwin icon to get a bash session
6. In bash, type: "cd viewer" and "build.sh"

Volume datasets can be viewed by first converting the raw data to the
PVM format using the "raw2pvm" command line tool. PVM files can be
converted back to raw data with the complementary "pvm2raw" tool which
also prints information about the volume data. The PVM files can be
also converted to a stack of PGM images with the "pvm2pgm" tool.

As an example PVM dataset the famous Bucky Ball is contained within
the package. In order to view this dataset just type "v3 Bucky.pvm" on
the console.

With cmake we can also configure whether or not to use the dcmtk
library for reading DICOM images. To do so, check the BUILD_WITH_DCMTK
setting in the cmake configuration via "ccmake .". This requires the
dcmtk library to be installed. The recommended install procedure on
*nix is to build and install dcmtk 3.6.0 from source:

   ./configure; make; sudo make install-libs

To view a stack of DICOM files, we type:

   v3 "filename*.dcm"

On platforms without a graphics accelerator the cross-section viewer
"pvmplay" can be used alternatively.

The V^3 package also contains the tools "pvminfo" and "pvm2pvm" which
amend a volume with additional information and quantize a 16 bit
volume to 8 bit using a non-linear mapping.

Have fun,
Stefan
