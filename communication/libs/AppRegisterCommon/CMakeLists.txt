#When the project is built, create this library
add_library(AppRegister_Common_Lib STATIC
    "src/Semaphore.cc")

#Add to the include path
target_include_directories(AppRegister_Common_Lib PUBLIC "include")
