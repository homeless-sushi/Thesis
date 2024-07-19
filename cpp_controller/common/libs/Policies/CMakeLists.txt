# When the project is built, create this library
add_library(CONTROLLER_Policies_LIB)

# Add sources
target_sources(CONTROLLER_Policies_LIB
    PRIVATE
        "src/SetConfigurationPolicy.cc"
        "src/PythonPolicyController.cc"
        "src/MargotOnlyPolicy.cc"
)

# Add to the include path
target_include_directories(CONTROLLER_Policies_LIB 
    PUBLIC
        "include"
)

target_link_libraries(CONTROLLER_Policies_LIB PUBLIC CONTROLLER_Utils_LIB)

# Add Runtime Manager dependency
target_link_libraries(CONTROLLER_Policies_LIB PUBLIC AppRegister_Common_Lib)
target_link_libraries(CONTROLLER_Policies_LIB PUBLIC AppRegister_Server_Lib)

# Add ZeroMQ and zmqpp dependency
target_link_libraries(CONTROLLER_Policies_LIB PUBLIC zmq)
target_link_libraries(CONTROLLER_Policies_LIB PUBLIC zmqpp)
