# Add the controller executable
add_executable(MargotOnlyController
    "src/main.cc"
)

# Rename the executable file
set_target_properties(MargotOnlyController 
    PROPERTIES 
        OUTPUT_NAME "RtrmController")

# Add Policies
target_link_libraries(MargotOnlyController PUBLIC CONTROLLER_Policies_LIB)

# Add Runtime Manager dependency
target_link_libraries(MargotOnlyController PUBLIC AppRegister_Common_Lib)
target_link_libraries(MargotOnlyController PUBLIC AppRegister_Server_Lib)

# Add Boost Libraries
target_include_directories(MargotOnlyController PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(MargotOnlyController PUBLIC ${Boost_LIBRARIES})