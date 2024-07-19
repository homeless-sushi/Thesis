# Set CMake Version
cmake_minimum_required(VERSION 3.7...3.25)

# Set CMake Policy (automatically set to version if VERSION >= 3.12)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

#Project name
project(RtrmCommunication)

#Run the CMake of libs/AppRegisterCommon
add_subdirectory("libs/AppRegisterCommon")

#Run the CMake of libs/AppRegisterServer
add_subdirectory("libs/AppRegisterServer")

#Run the CMake of libs/AppRegisterClient
add_subdirectory("libs/AppRegisterClient")
