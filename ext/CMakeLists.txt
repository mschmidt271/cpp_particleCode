include(ExternalProject)
include(GNUInstallDirs)
include_directories(${PROJECT_BINARY_DIR}/include)

# Create canonical top-level directories in the build directory.
foreach(dir include;lib;bin;share)
  set(top_level_dir ${PROJECT_BINARY_DIR}/${dir})
  if (NOT EXISTS ${top_level_dir})
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${top_level_dir})
  endif()
endforeach()

# Keep track of whether we've updated our submodules.
set(SUBMODULES_UPDATED FALSE)
macro(update_submodules)
  if (NOT SUBMODULES_UPDATED)
    message(STATUS "Updating git submodules recursively...")
    execute_process(COMMAND ${GIT} submodule update --init --recursive
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
    set(SUBMODULES_UPDATED TRUE)
  endif()
endmacro()

#----------------------------------------------------------------------------
# Dynamic loading library
#----------------------------------------------------------------------------
if (NOT APPLE)
  find_library(DL_LIBRARY libdl${CMAKE_SHARED_LIBRARY_SUFFIX} REQUIRED)
  add_library(dl SHARED IMPORTED GLOBAL)
  set_target_properties(dl PROPERTIES IMPORTED_LOCATION ${DL_LIBRARY})
  set(EXT_LIBRARIES dl) # Required by kokkoscore.
endif()

# FIXME: this should be done in a setup script, referencing a machine file
if (DEFINED ENV{KOKKOS_LIBRARY})
  set(KOKKOS_INCLUDE_DIR $ENV{KOKKOS_INCDIR})
  set(KOKKOS_LIBRARY_DIR $ENV{KOKKOS_LIBDIR})
  set(KOKKOS_LIBRARY "$ENV{KOKKOS_LIBDIR}/$ENV{KOKKOS_LIBRARY}")
  message(STATUS "Kokkos found at ${KOKKOS_LIBRARY}")
else()
  set(KOKKOS_INCLUDE_DIR "${PROJECT_BINARY_DIR}/include")
  set(KOKKOS_LIBRARY_DIR "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
  set(KOKKOS_LIBRARY "${KOKKOS_LIBRARY_DIR}/libkokkoscore.a")
  # set(KOKKOS_LIBDIR $ENV{KOKKOS_LIBDIR})
endif()

add_library(kokkos STATIC IMPORTED GLOBAL)
set_target_properties(kokkos PROPERTIES IMPORTED_LOCATION ${KOKKOS_LIBRARY})
if (NOT EXISTS ${KOKKOS_LIBRARY})
  message(STATUS "Building Kokkos as external project")
  update_submodules()
  if (CMAKE_BUILD_TYPE STREQUAL Release)
    set(KOKKOS_CMAKE_OPTS ${KOKKOS_CMAKE_OPTS}
                          -DKokkos_ENABLE_DEBUG=FALSE
                          -DKokkos_ENABLE_AGGRESSIVE_VECTORIZATION=ON)
  else()
    set(KOKKOS_CMAKE_OPTS ${KOKKOS_CMAKE_OPTS}
                          -DKokkos_ENABLE_DEBUG=TRUE
                          -DKokkos_ENABLE_AGGRESSIVE_VECTORIZATION=OFF)
  endif()
  if (PARPT_USE_OPENMP)
    set(KOKKOS_CMAKE_OPTS ${KOKKOS_CMAKE_OPTS}
                          -DKokkos_ENABLE_OPENMP=ON
                          -DKokkos_ENABLE_SERIAL=ON)
  elseif(PARPT_USE_CUDA)
    message(STATUS "GPU_DEVICE_ARCH_FLAG = $ENV{GPU_DEVICE_ARCH_FLAG}")
    set(KOKKOS_CMAKE_OPTS ${KOKKOS_CMAKE_OPTS}
                          -DKokkos_ENABLE_CUDA=ON
                          -DKokkos_ENABLE_OPENMP=ON
                          -DKokkos_ENABLE_SERIAL=ON
                          $ENV{GPU_DEVICE_ARCH_FLAG}
                          -DKokkos_ENABLE_CUDA_LAMBDA=ON)
    message(STATUS "KOKKOS_CMAKE_OPTS = ${KOKKOS_CMAKE_OPTS}")
  else()
    set(KOKKOS_CMAKE_OPTS ${KOKKOS_CMAKE_OPTS}
                          -DKokkos_ENABLE_OPENMP=OFF
                          -DKokkos_ENABLE_CUDA=OFF
                          -DKokkos_ENABLE_SERIAL=ON)

  endif()
  set(KOKKOS_CMAKE_OPTS ${KOKKOS_CMAKE_OPTS}
                        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}
                        -DKokkos_ENABLE_PTHREAD=OFF
                        -DKokkos_ENABLE_SERIAL=ON
                        -DKokkos_ENABLE_DEPRECATED_CODE=OFF)
  ExternalProject_Add(kokkos_proj
                      PREFIX ${CMAKE_CURRENT_BINARY_DIR}/kokkos
                      SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/kokkos
                      BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/kokkos
                      INSTALL_DIR ${PROJECT_BINARY_DIR}
                      CMAKE_ARGS ${KOKKOS_CMAKE_OPTS}
                      BUILD_COMMAND make -j
                      LOG_CONFIGURE TRUE
                      LOG_BUILD TRUE
                      INSTALL_COMMAND make install
                      LOG_INSTALL TRUE)

  add_dependencies(kokkos kokkos_proj)
endif()
list(APPEND EXT_INCDIRS ${KOKKOS_INCLUDE_DIR})
set(EXT_LIBRARIES ${EXT_LIBRARIES};kokkos)

# FIXME: this should be done in a setup script, referencing a machine file
if (DEFINED ENV{KOKKOSKERNELS_LIBRARY})
  set(KOKKOSKERNELS_INCLUDE_DIR $ENV{KOKKOSKERNELS_INCDIR})
  set(KOKKOSKERNELS_LIBRARY_DIR $ENV{KOKKOSKERNELS_LIBDIR})
  set(KOKKOSKERNELS_LIBRARY "$ENV{KOKKOSKERNELS_LIBDIR}/$ENV{KOKKOSKERNELS_LIBRARY}")
  message(STATUS "Kokkos Kernels found at ${KOKKOSKERNELS_LIBRARY}")
else()
  set(KOKKOSKERNELS_INCLUDE_DIR "${PROJECT_BINARY_DIR}/include")
  set(KOKKOSKERNELS_LIBRARY_DIR "${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
  set(KOKKOSKERNELS_LIBRARY ${KOKKOSKERNELS_LIBRARY_DIR}/libkokkoskernels.a)
  # set(KOKKOS_LIBDIR $ENV{KOKKOS_LIBDIR})
endif()

add_library(kokkoskernels STATIC IMPORTED GLOBAL)
set_target_properties(kokkoskernels PROPERTIES IMPORTED_LOCATION ${KOKKOSKERNELS_LIBRARY})
if (NOT EXISTS ${KOKKOSKERNELS_LIBRARY})
  message(STATUS "Building Kokkos Kernels as external project")
  update_submodules()
  set(KOKKOS_CONFIG_DIR ${KOKKOS_LIBRARY_DIR}/cmake/Kokkos)
  set(KOKKOSKERNELS_CMAKE_OPTS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                               -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}
                               -DKokkos_DIR=${KOKKOS_CONFIG_DIR}
                               -DKokkosKernels_ENABLE_EXPERIMENTAL=OFF
                               -DKokkosKernels_ENABLE_TESTS=OFF)
  ExternalProject_Add(kokkoskernels_proj
                      PREFIX ${CMAKE_CURRENT_BINARY_DIR}/kokkos-kernels
                      SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/kokkos-kernels
                      BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/kokkos-kernels
                      INSTALL_DIR ${PROJECT_BINARY_DIR}
                      CMAKE_ARGS ${KOKKOSKERNELS_CMAKE_OPTS}
                      DEPENDS kokkos
                      BUILD_COMMAND make -j
                      LOG_CONFIGURE TRUE
                      LOG_BUILD TRUE
                      INSTALL_COMMAND make install
                      LOG_INSTALL TRUE)

  add_dependencies(kokkoskernels kokkoskernels_proj)
endif()
list(APPEND EXT_INCDIRS ${KOKKOSKERNELS_INCLUDE_DIR})
set(EXT_LIBRARIES ${EXT_LIBRARIES};kokkoskernels)

# FIXME: this should be done in a setup script, referencing a machine file
if (DEFINED ENV{YAML_CPP_LIBRARY})
  set(YAML_CPP_INCLUDE_DIR $ENV{YAML_CPP_INCDIR})
  set(YAML_CPP_LIBRARY_DIR $ENV{YAML_CPP_LIBDIR})
  set(YAML_CPP_LIBRARY "$ENV{YAML_CPP_LIBDIR}/$ENV{YAML_CPP_LIBRARY}")
  message(STATUS "yaml-cpp found at ${YAML_CPP_LIBRARY}")
else()
  set(YAML_CPP_INCLUDE_DIR "${PROJECT_BINARY_DIR}/include")
  set(YAML_CPP_LIBRARY_DIR "${PROJECT_BINARY_DIR}/lib")
  set(YAML_CPP_LIBRARY ${YAML_CPP_LIBRARY_DIR}/libyaml-cpp.a)
  set(YAML_CPP_LIBDIR $ENV{YAML_CPP_LIBDIR})
endif()

add_library(yaml_cpp STATIC IMPORTED GLOBAL)
set_target_properties(yaml_cpp PROPERTIES IMPORTED_LOCATION ${YAML_CPP_LIBRARY})
if (NOT EXISTS ${YAML_CPP_LIBRARY})
  message(STATUS "Building yaml_cpp as external project")
  update_submodules()
  set(YAML_CPP_CMAKE_OPTS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                          -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                          -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}
                          -DCMAKE_VERBOSE_MAKEFILE=OFF
                          -DYAML_CPP_BUILD_TESTS=OFF)
  ExternalProject_Add(yaml_cpp_proj
                      PREFIX ${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp
                      SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/yaml-cpp
                      BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp
                      INSTALL_DIR ${PROJECT_BINARY_DIR}
                      CMAKE_ARGS ${YAML_CPP_CMAKE_OPTS}
                      BUILD_COMMAND make -j
                      LOG_CONFIGURE TRUE
                      LOG_BUILD TRUE
                      INSTALL_COMMAND make install
                      LOG_INSTALL TRUE)

  add_dependencies(yaml_cpp yaml_cpp_proj)
endif()
list(APPEND EXT_INCDIRS ${YAML_CPP_INCLUDE_DIR})
set(EXT_LIBRARIES ${EXT_LIBRARIES};yaml_cpp)

# Add all the libraries to the build system at large.
set(LIBRARIES ${EXT_LIBRARIES} PARENT_SCOPE)
list(REMOVE_DUPLICATES EXT_INCDIRS)
set(PARPT_INCDIRS ${PARPT_INCDIRS};${EXT_INCDIRS} PARENT_SCOPE)

if (${KOKKOS_LIBRARY_DIR} MATCHES ${PROJECT_BINARY_DIR}) # we built kokkos
  install(FILES ${KOKKOS_LIBRARY} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
if (${KOKKOSKERNELS_LIBRARY_DIR} MATCHES ${PROJECT_BINARY_DIR}) # we built kokkos kernels
  install(FILES ${KOKKOSKERNELS_LIBRARY} DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
if (${YAML_CPP_LIBRARY_DIR} MATCHES ${PROJECT_BINARY_DIR}) # we built yaml-cpp
  # note that yaml-cpp is hard-coded to install to "lib"
  install(FILES ${YAML_CPP_LIBRARY} DESTINATION lib)
endif()

