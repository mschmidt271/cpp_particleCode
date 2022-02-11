include_directories(${PARPT_INCDIRS})
# add utilities.cpp as a library
add_library(Utils utilities.cpp)
add_dependencies(Utils yaml_cpp kokkos kokkoskernels arborx)
# copy the plotting directory to install dir
install(DIRECTORY plotting DESTINATION ${CMAKE_INSTALL_PREFIX})
