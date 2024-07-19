# Add the controller executable
add_executable(PythonPolicyController
    "src/main.cc"
)

# Rename the executable file
set_target_properties(PythonPolicyController 
    PROPERTIES 
        OUTPUT_NAME "RtrmController")

# Add Policies
target_link_libraries(PythonPolicyController PUBLIC CONTROLLER_Policies_LIB)

# Add Runtime Manager dependency
target_link_libraries(PythonPolicyController PUBLIC AppRegister_Common_Lib)
target_link_libraries(PythonPolicyController PUBLIC AppRegister_Server_Lib)

# Add Boost Libraries
target_include_directories(PythonPolicyController PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(PythonPolicyController PUBLIC ${Boost_LIBRARIES})