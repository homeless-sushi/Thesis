# Add the controller executable
add_executable(SetConfigurationController
    "src/main.cc"
)

# Rename the executable file
set_target_properties(SetConfigurationController 
    PROPERTIES 
        OUTPUT_NAME "RtrmController")

# Add Policies
target_link_libraries(SetConfigurationController PUBLIC CONTROLLER_Policies_LIB)

# Add Runtime Manager dependency
target_link_libraries(SetConfigurationController PUBLIC AppRegister_Common_Lib)
target_link_libraries(SetConfigurationController PUBLIC AppRegister_Server_Lib)

# Add Boost Libraries
target_include_directories(SetConfigurationController PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(SetConfigurationController PUBLIC ${Boost_LIBRARIES})