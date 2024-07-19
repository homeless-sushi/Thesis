#When the project is built, create this library
add_library(AppRegister_Client_Lib STATIC
    "src/AppRegister.cc"
    "src/Utils.cc"
)

#Add to the include path
target_include_directories(AppRegister_Client_Lib PUBLIC "include")

#Add the these libraries as dipendencies
target_link_libraries(AppRegister_Client_Lib PUBLIC AppRegister_Common_Lib)
