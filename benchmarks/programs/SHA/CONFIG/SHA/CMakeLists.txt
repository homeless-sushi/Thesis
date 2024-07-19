# When the project is built, build this library
add_executable(SHA_Config
    "src/sha.cc"
)

# Rename the executable file
set_target_properties(SHA_Config
    PROPERTIES 
        OUTPUT_NAME "SHA")

# Add sources to the target
#target_sources(SHA_Config
#    PRIVATE
#        "src/sha.cc"
#)

# Add to the include path of the target
target_include_directories(SHA_Config 
    PUBLIC 
        "include"
)
target_link_libraries(SHA_Config
    PUBLIC
        SHA_Sha_LIB
)

# Add Boost Libraries
target_include_directories(SHA_Config PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(SHA_Config PUBLIC ${Boost_LIBRARIES})

# Add runtime manager dependencies
target_link_libraries(SHA_Config PUBLIC AppRegister_Common_Lib)
target_link_libraries(SHA_Config PUBLIC AppRegister_Client_Lib)
