# Set CMake Version
cmake_minimum_required(VERSION 3.7...3.22)

# Set CMake Policy (automatically set to version if VERSION >= 3.12)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# Project name
project(
    NBODY_ControllerMargot
    LANGUAGES CUDA CXX)

# Enable debug with cuda-gdb
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -g -G") 
endif()

# Enable OpenMP on NVCC
#set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -Xcompiler -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

# Find Boost Libraries
set(BOOST_ROOT $ENV{boost_DIR})
find_package(Boost REQUIRED
    COMPONENTS program_options)
# When using Boost 1.73
## find_package(Boost REQUIRED
##    COMPONENTS ALLL)

# Find ControllerMargot
set(eclipse-paho-mqtt-c_DIR $ENV{paho_DIR}) # cannot set an env variable containing hyphen
find_package(margot_heel_generator REQUIRED)

# Run the CMake of these subdirectories
add_subdirectory("NBODY")

# Add these out-of-tree subdirectories
set(NBODY_COMMON_DIR "../common")
add_subdirectory(${NBODY_COMMON_DIR} "common/nbody")
set(PROJECT_COMMON_DIR "../../../common")
add_subdirectory(${PROJECT_COMMON_DIR} "common/project")

# Add runtime manager subdirectories and dependencies
set(RTRM_COMM_DIR $ENV{rtrm_communication_DIR})
add_subdirectory(${RTRM_COMM_DIR} "rtrm")
