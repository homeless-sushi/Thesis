#When the project is built, create this library
add_library(AppRegister_Server_Lib STATIC
    "src/CGroupUtils.cc"
    "src/AppRegister.cc"
    "src/AppData.cc"
    "src/AppUtils.cc"
    "src/App.cc"
    "src/Policy.cc"
    "src/Utilization.cc"
    "src/Frequency.cc"
    "src/Sensors.cc")

#Add to the include path
target_include_directories(AppRegister_Server_Lib PUBLIC "include")

#Add the these libraries as dipendencies
target_link_libraries(AppRegister_Server_Lib PUBLIC AppRegister_Common_Lib)
target_link_libraries(AppRegister_Server_Lib PUBLIC cgroup)
