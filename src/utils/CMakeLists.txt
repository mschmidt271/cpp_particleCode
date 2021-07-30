# need this include dir for yaml-cpp
include_directories(${PARTICLERW_INCDIRS})
# add utilities.cpp as a library
add_library(Utils utilities.cpp)
add_dependencies(Utils yaml_cpp)
# copy the plotting directory to install dir
install(DIRECTORY plotting DESTINATION ${CMAKE_INSTALL_PREFIX})
