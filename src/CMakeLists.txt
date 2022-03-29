# Precision of floating point numbers.
if (PARPT_PRECISION STREQUAL "single")
  set(PARPT_REAL_TYPE "float")
  set(PARPT_DOUBLE_PRECISION 0)
elseif(PARPT_PRECISION STREQUAL "double")
  set(PARPT_REAL_TYPE "double")
  set(PARPT_DOUBLE_PRECISION 1)
elseif(PARPT_PRECISION)
  message(FATAL_ERROR "Invalid PARPT_PRECISION: ${PARPT_PRECISION} (use 'single' or 'double')")
else()
  message(FATAL_ERROR "PARPT_PRECISION not set (use 'single' or 'double')")
endif()
message(STATUS "Using ${PARPT_PRECISION} precision floating point numbers")

# generate a configure file
  # currently, just provides the install prefix
  # NOTE: here, and below, PROJECT_SOURCE_DIR refers to the root directory,
          # since that's where the project() call is
configure_file("${PROJECT_SOURCE_DIR}/src/utils/parPT_config.hpp.in"
               "${PROJECT_BINARY_DIR}/src/utils/parPT_config.hpp"
               @ONLY
               )

message(STATUS "PARPT_REAL_TYPE = ${PARPT_REAL_TYPE}")

list(APPEND PARPT_INCDIRS;
            ${PROJECT_SOURCE_DIR}/src/utils;
            ${PROJECT_BINARY_DIR}/src/utils)

# add the utils directory, with the utilities library and plotting directory
add_subdirectory(utils)

# # add the external kokkos library
# find_library(kk kokkoscore HINTS $ENV{KOKKOS_LIBDIR})

# # Report the installation prefix.
# message(STATUS "Kokkos found at ${kk}")

# # add the external yaml-cpp library
# find_library(ycpp yaml-cpp HINTS $ENV{YAMLCPP_LIBDIR})

# # Report the installation prefix.
# message(STATUS "yaml-cpp found at ${ycpp}")

# Report the installation prefix.
message(STATUS "Installation prefix is ${CMAKE_INSTALL_PREFIX}")

# add the executable and link the libraries
add_executable(parPT parPT.cpp)

if(PARPT_USE_CUDA)
  find_package(CUDA)
  cuda_add_cublas_to_target(parPT)
  list(APPEND LIBRARIES; ${CUDA_cusparse_LIBRARY})
endif()

# add_dependencies(PARPT_LIBS)

set(PARPT_LIBS Utils)
set(LIBRARIES ${LIBRARIES};${PARPT_LIBS})

# add the unit testing subdirectory (FIXME: automate this one day...)
# NOTE: in order for this to see the PARPT_LIBS, this has be be after they are
# appended to LIBRARIES b^.^d
add_subdirectory(unit_test)

target_link_libraries(parPT
                      ${LIBRARIES}
                      )

message(STATUS "Libraries = ${LIBRARIES}")

# add the include directories
target_include_directories(parPT PUBLIC
                           ${PARPT_INCDIRS}
                           # ${PROJECT_SOURCE_DIR}/src
                          )

set(tests "MT_only" ; "RW_only" ; "RWMT")

# FIXME: is this necessary?
install(TARGETS parPT Utils)
install(DIRECTORY data DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY tests DESTINATION ${CMAKE_INSTALL_PREFIX})
# install(DIRECTORY unit_test DESTINATION ${CMAKE_INSTALL_PREFIX})
foreach(test ${tests})
  install(PROGRAMS tests/${test}/run_${test}.sh DESTINATION ${CMAKE_INSTALL_PREFIX}/tests/${test})
endforeach()
# NOTE: PROGRAMS (vs. FILES) will automatically make the shell script executable
install(PROGRAMS run.sh DESTINATION ${CMAKE_INSTALL_PREFIX})
install(PROGRAMS plot.sh DESTINATION ${CMAKE_INSTALL_PREFIX})
