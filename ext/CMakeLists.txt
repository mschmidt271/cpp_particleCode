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

# FIXME: this should be done in a setup script, referencing a machine file
if (DEFINED ENV{KOKKOS_LIBRARY})
  set(KOKKOS_INCLUDE_DIR $ENV{KOKKOS_INCDIR})
  set(KOKKOS_LIBRARY_DIR $ENV{KOKKOS_LIBDIR})
  set(KOKKOS_LIBRARY "$ENV{KOKKOS_LIBDIR}/$ENV{KOKKOS_LIBRARY}")
  message(STATUS "Kokkos found at ${KOKKOS_LIBRARY}")
else()
  set(KOKKOS_INCLUDE_DIR "${PROJECT_BINARY_DIR}/include")
  set(KOKKOS_LIBRARY_DIR "${PROJECT_BINARY_DIR}/lib")
  set(KOKKOS_LIBRARY ${KOKKOS_LIBRARY_DIR}/libkokkoscore.a)
  set(KOKKOS_LIBDIR $ENV{KOKKOS_LIBDIR})
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
  set(KOKKOS_CMAKE_OPTS -DCMAKE_CXX_COMPILER=${CMAKE_C_COMPILER}
                        -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}
                        -DKokkos_ENABLE_PTHREAD=OFF
                        -DKokkos_ENABLE_OPENMP=OFF
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
  set(KOKKOSKERNELS_LIBRARY_DIR "${PROJECT_BINARY_DIR}/lib")
  set(KOKKOSKERNELS_LIBRARY ${KOKKOSKERNELS_LIBRARY_DIR}/libkokkoskernels.a)
  set(KOKKOS_LIBDIR $ENV{KOKKOS_LIBDIR})
endif()

add_library(kokkoskernels STATIC IMPORTED GLOBAL)
set_target_properties(kokkoskernels PROPERTIES IMPORTED_LOCATION ${KOKKOSKERNELS_LIBRARY})
if (NOT EXISTS ${KOKKOSKERNELS_LIBRARY})
  message(STATUS "Building Kokkos Kernels as external project")
  update_submodules()
  set(KOKKOS_CONFIG_DIR ${KOKKOS_LIBDIR}/cmake/Kokkos)
  set(KOKKOSKERNELS_CMAKE_OPTS -DCMAKE_CXX_COMPILER=${CMAKE_C_COMPILER}
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

# Add all the libraries to the build system at large.
set(LIBRARIES ${EXT_LIBRARIES} PARENT_SCOPE)
list(REMOVE_DUPLICATES EXT_INCDIRS)
set(PARTICLERW_INCDIRS ${PARTICLERW_INCDIRS};${EXT_INCDIRS} PARENT_SCOPE)

if (${KOKKOS_LIBRARY_DIR} MATCHES ${PROJECT_BINARY_DIR}) # we built kokkos
  install(FILES ${KOKKOS_LIBRARY} DESTINATION lib)
endif()
if (${KOKKOSKERNELS_LIBRARY_DIR} MATCHES ${PROJECT_BINARY_DIR}) # we built kokkos kernels
  install(FILES ${KOKKOSKERNELS_LIBRARY} DESTINATION lib)
endif()

