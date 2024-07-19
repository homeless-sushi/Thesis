# Set CMake Version
cmake_minimum_required(VERSION 3.7...3.25)

# Set CMake Policy (automatically set to version if VERSION >= 3.12)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

#Project name
project(RtrmControllers)

# Find Boost Libraries
set(BOOST_ROOT $ENV{boost_DIR})
find_package(Boost REQUIRED
    COMPONENTS program_options)

# Run the CMake of these subdirectories
add_subdirectory("controllers")
add_subdirectory("common")

# Add runtime manager subdirectories and dependencies
set(RTRM_COMM_DIR $ENV{rtrm_communication_DIR})
add_subdirectory(${RTRM_COMM_DIR} "rtrm")
