
# generate a configure file
  # currently, just provides the install prefix
  # NOTE: here, and below, PROJECT_SOURCE_DIR refers to the root directory,
          # since that's where the project() call is
configure_file("${PROJECT_SOURCE_DIR}/src/utils/ParticleRW_config.h.in"
               "${PROJECT_BINARY_DIR}/src/utils/ParticleRW_config.h"
               )

# add the utils directory, with the utilities library and plotting directory
add_subdirectory(utils)

# add the external kokkos library
find_library(kk kokkoscore HINTS $ENV{KOKKOS_LIBDIR})

# Report the installation prefix.
message(STATUS "Kokkos found at ${kk}")

# add the external kokkos-kernels library
# find_library(kkern kokkoskernels HINTS $ENV{KKERNELS_LIBDIR})

# # Report the installation prefix.
# message(STATUS "Kokkos Kernels found at ${kkern}")

# add the external yaml-cpp library
find_library(ycpp yaml-cpp HINTS $ENV{YAMLCPP_LIBDIR})

# Report the installation prefix.
message(STATUS "yaml-cpp found at ${ycpp}")

# Report the installation prefix.
message(STATUS "Installation prefix is ${CMAKE_INSTALL_PREFIX}")

# add the executable and link the libraries
add_executable(ParticleRW particleRW.cpp)
target_link_libraries(ParticleRW
                      ${kk}
                      ${LIBRARIES}
                      ${ycpp}
                      Utils
                      )

list(APPEND PARTICLERW_INCDIRS;
            ${PROJECT_SOURCE_DIR}/src/utils;
            ${PROJECT_BINARY_DIR}/src/utils)

# add the include directories
target_include_directories(ParticleRW PUBLIC
                           $ENV{YAMLCPP_INCDIR}
                           $ENV{KOKKOS_INCDIR}
                           ${PARTICLERW_INCDIRS}
                           # this is needed for yaml-cpp
                           "/usr/local/include"
                          )

install(TARGETS ParticleRW Utils)
install(DIRECTORY data DESTINATION ${CMAKE_INSTALL_PREFIX})
# NOTE: PROGRAMS (vs. FILES) will automatically make the shell script executable
install(PROGRAMS run.sh DESTINATION ${CMAKE_INSTALL_PREFIX})
