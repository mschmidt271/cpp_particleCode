include_directories(${PARPT_INCDIRS})
# add utilities.cpp as a library
add_library(Utils version_info.cpp
                  containers.cpp
                  mass_transfer.cpp
                  utilities.cpp)
set(Utils PARENT_SCOPE)
# add_dependencies(Utils yaml_cpp kokkos kokkoskernels arborx spdlog version_info)
# copy the plotting directory to install dir
install(DIRECTORY plotting DESTINATION ${CMAKE_INSTALL_PREFIX})
# note that the type_defs.hpp header is found by adding this directory to the
# target_include_directories for the parPT project
