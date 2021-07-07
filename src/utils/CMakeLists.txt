# need this include dir for yaml-cpp
include_directories("/usr/local/include")
# add utilities.cpp as a library
add_library(Utils utilities.cpp)
# copy the plotting directory to install dir
install(DIRECTORY plotting DESTINATION ${CMAKE_INSTALL_PREFIX})
