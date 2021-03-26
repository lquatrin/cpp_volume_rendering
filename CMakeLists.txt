cmake_minimum_required(VERSION 3.9.2 FATAL_ERROR)
project(dev_vs)

# ensure c++11
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Setting MSVC flags")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHc /std:c++latest")

message(${CMAKE_SYSTEM_PROCESSOR})
message(${CMAKE_SIZEOF_VOID_P}) # 8 for 64 bit and 4 for 32 bit
#message(${PROJECTNAME_ARCHITECTURE})
message(${CMAKE_CURRENT_BINARY_DIR}) 

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_EXTERNAL_DIRECTORY       "${CMAKE_SOURCE_DIR}/external")
#CMAKE_CURRENT_BINARY_DIR

# -- DEFINES
# Using suggestion on using MAKE_STR macro at C++ code from "naoyam"
# . https://github.com/LLNL/lbann/issues/117 
set(PATH_TO_RESOURCES ${CMAKE_SOURCE_DIR}/../resources/)
add_definitions(-DCMAKE_PATH_TO_RESOURCES=${PATH_TO_RESOURCES})

# adding libraries folder
add_subdirectory(libs)

# add application
add_subdirectory(cppvolrend)

# cmake -G "Visual Studio 15 2017 Win64"
# https://cognitivewaves.wordpress.com/cmake-and-visual-studio/