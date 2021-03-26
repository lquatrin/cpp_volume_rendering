add_library(math_utils STATIC convexhull.cpp           convexhull.h
                              geometry.cpp             geometry.h
                              gfilter.cpp              gfilter.h
                                                       matrix.h
                                                       matrix4.h
                              utils.cpp                utils.h
                              )

include_directories(${CMAKE_SOURCE_DIR}/include)
add_definitions(-DEXPMODULE)