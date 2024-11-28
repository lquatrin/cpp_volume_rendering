## Adding a new volume renderer

All volume rendering algorithms are found in `cppvolrend/structured`. Assume we want to create a new renderer, which visualizes isosurfaces. We call it a *ray-casting 1-pass isosurface renderer*, or short `rc1piso`. These steps need to be taken to add it:

* Create a new subfolder `cppvolrend/rc1piso` and add some files:
* * Add the C++ files named `rc1pisorenderer.cpp` and `rc1pisorenderer.h`. Their content could come from one of the other algorithms, or be based on `volrendernull.cpp/h`.
* * Add a compute shader called `ray_marching_1p_iso.comp`.
* Edit `structured/CMakeLists.txt` to add the new C++ files in the call to `add_executable()`. Follow the structure of the existing text there.
* Run CMake. The files should now be part of your buildsystem and appear in your IDE.
* Include the new renderer in the list of renderers available to the program by adding a few lines in `structured/main.cpp`:

```c++
// near the beginning of main.cpp ...
#include "structured/rc1piso/rc1pisorenderer.h"

// ... and in the main function
int main (int argc, char **argv)
{
    RenderingManager::Instance()->AddVolumeRenderer(new RayCasting1PassIso());
}
```

You can now compile and test your new renderer.

