# When the project is built, create this library
add_library(CONTROLLER_Utils_LIB)

# Add sources
target_sources(CONTROLLER_Utils_LIB
    PRIVATE
        "src/ConfigInfo.cc"
)

# Add to the include path
target_include_directories(CONTROLLER_Utils_LIB 
    PUBLIC
        "include"
)

# Add Runtime Manager dependency
target_link_libraries(CONTROLLER_Utils_LIB PUBLIC AppRegister_Common_Lib)
target_link_libraries(CONTROLLER_Utils_LIB PUBLIC AppRegister_Server_Lib)
