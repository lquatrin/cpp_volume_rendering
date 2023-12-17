# Visualization Library focused on volume related things
# . Volume Rendering

set(V_LIB_VOLVIS_UTILS_SHADER_DIR ${CMAKE_SOURCE_DIR}/libs/volvis_utils/shader/)
add_definitions(-DCMAKE_VOLVIS_UTILS_PATH_TO_SHADER=${V_LIB_VOLVIS_UTILS_SHADER_DIR})

add_library(volvis_utils STATIC camerastatelist.cpp        camerastatelist.h
                                datamanager.cpp            datamanager.h
                                generalizedsampling.cpp    generalizedsampling.h
                                gridvolume.cpp             gridvolume.h
                                imagefilter.cpp            imagefilter.h
                                lightsourcelist.cpp        lightsourcelist.h
                                reader.cpp                 reader.h
                                renderingparameters.cpp    renderingparameters.h
                                structuredgridvolume.cpp   structuredgridvolume.h
                                transferfunction.cpp       transferfunction.h
                                transferfunction1d.cpp     transferfunction1d.h
                                unstructuredgridvolume.cpp unstructuredgridvolume.h
                                utils.cpp                  utils.h
                                tetrahedron.cpp            tetrahedron.h
                                dataprovider.cpp           dataprovider.h)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_definitions(-DEXPMODULE)
include_directories(${CMAKE_SOURCE_DIR}/libs)
include_directories(${CMAKE_SOURCE_DIR}/lib)

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

target_link_libraries(volvis_utils debug file_utils)
target_link_libraries(volvis_utils debug math_utils)
target_link_libraries(volvis_utils debug gl_utils)
target_link_libraries(volvis_utils debug vis_utils)

target_link_libraries(volvis_utils optimized file_utils)
target_link_libraries(volvis_utils optimized math_utils)
target_link_libraries(volvis_utils optimized gl_utils)
target_link_libraries(volvis_utils optimized vis_utils)
                      
# add dependency
add_dependencies(volvis_utils file_utils)
add_dependencies(volvis_utils math_utils)
add_dependencies(volvis_utils gl_utils)
add_dependencies(volvis_utils vis_utils)