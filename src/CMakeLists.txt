
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
find_library(kkern kokkoskernels HINTS $ENV{KKERNELS_LIBDIR})

# Report the installation prefix.
message(STATUS "Kokkos Kernels found at ${kkern}")

# add the external obenblas library
find_library(oblas openblas HINTS $ENV{OPENBLAS_LIBDIR})

# Report the installation prefix.
message(STATUS "OpenBlas found at ${oblas}")

# add the external yaml-cpp library
find_library(ycpp yaml-cpp HINTS $ENV{YAMLCPP_LIBDIR})

# Report the installation prefix.
message(STATUS "yaml-cpp found at ${ycpp}")

# # add the external lapack library
# find_library(lpack lapack)

# # Report the installation prefix.
# message(STATUS "Lapack found at ${lpack}")

# FIXME: probably need to track down openblas/lapack in order to use kkernels blas functions

# Report the installation prefix.
message(STATUS "Installation prefix is ${CMAKE_INSTALL_PREFIX}")

# add the executable and link the libraries
add_executable(ParticleRW particleRW.cpp)
target_link_libraries(ParticleRW
                      Utils
                      ${kk}
                      ${kkern}
                      ${oblas}
                      ${ycpp}
                      )
# add the include directories
target_include_directories(ParticleRW PUBLIC
                           # this is needed for the config file
                           ${PROJECT_BINARY_DIR}/src/utils
                           # this is needed for utils
                           "${PROJECT_SOURCE_DIR}/src/utils"
                           $ENV{KOKKOS_INCDIR}
                           $ENV{KKERNELS_INCDIR}
                           # this is needed for yaml-cpp
                           # "/usr/local/include"
                           $ENV{YAMLCPP_INCDIR}
                          )

install(TARGETS ParticleRW Utils)
install(DIRECTORY data DESTINATION ${CMAKE_INSTALL_PREFIX})
# NOTE: PROGRAMS (vs. FILES) will automatically make the shell script executable
install(PROGRAMS run.sh DESTINATION ${CMAKE_INSTALL_PREFIX})
