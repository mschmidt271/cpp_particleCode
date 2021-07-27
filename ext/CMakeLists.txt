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
if (DEFINED ENV{OPENBLAS_LIBRARY})
  set(OPENBLAS_INCLUDE_DIR $ENV{OPENBLAS_INCDIR})
  set(OPENBLAS_LIBRARY_DIR $ENV{OPENBLAS_LIBDIR})
  set(OPENBLAS_LIBRARY "$ENV{OPENBLAS_LIBDIR}/$ENV{OPENBLAS_LIBRARY}")
else()
  set(OPENBLAS_INCLUDE_DIR "${PROJECT_BINARY_DIR}/include")
  set(OPENBLAS_LIBRARY_DIR "${PROJECT_BINARY_DIR}/lib")
  set(OPENBLAS_LIBRARY ${OPENBLAS_LIBRARY_DIR}/libopenblas.a)
endif()

set(KOKKOSKERNELS_INCLUDE_DIR ${PROJECT_BINARY_DIR}/include)
set(KOKKOSKERNELS_LIBRARY_DIR ${PROJECT_BINARY_DIR}/lib)
set(KOKKOSKERNELS_LIBRARY ${KOKKOSKERNELS_LIBRARY_DIR}/libkokkoskernels.a)
set(KOKKOS_LIBDIR $ENV{KOKKOS_LIBDIR})
message(STATUS "OPENBLAS_LIBRARY = ${OPENBLAS_LIBRARY}")

add_library(openblas STATIC IMPORTED GLOBAL)
set_target_properties(openblas PROPERTIES IMPORTED_LOCATION ${OPENBLAS_LIBRARY})
# if the library dir was set above (in the build directory), rather than passed
# in via environment/configure file, then we build
if (${OPENBLAS_LIBRARY_DIR} MATCHES "${PROJECT_BINARY_DIR}/lib")
  message(STATUS "Building OpenBLAS as external project")
  update_submodules()
  # set(OPENBLAS_OPTS NO_SHARED=1
  #                   PREFIX=${PROJECT_BINARY_DIR})
  # if (HAERO_DOUBLE_PRECISION)
    # set(OPENBLAS_OPTS ${OPENBLAS_OPTS} BUILD_DOUBLE=1)
  # else()
  #   set(OPENBLAS_OPTS ${OPENBLAS_OPTS} BUILD_SINGLE=1)
  # endif()
  set(OPENBLAS_CMAKE_OPTS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                          -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                          -DCMAKE_Fortran_COMPILER=${CMAKE_Fortran_COMPILER}
                          -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}
                          -DBUILD_DOUBLE=ON)
  ExternalProject_Add(openblas_proj
                      PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenBLAS
                      SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/OpenBLAS
                      BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/OpenBLAS
                      INSTALL_DIR ${PROJECT_BINARY_DIR}
                      # CONFIGURE_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/OpenBLAS ${CMAKE_CURRENT_BINARY_DIR}/OpenBLAS
                      CMAKE_ARGS ${OPENBLAS_CMAKE_OPTS}
                      BUILD_COMMAND make -j
                      LOG_CONFIGURE TRUE
                      LOG_BUILD TRUE
                      INSTALL_COMMAND make install
                      LOG_INSTALL TRUE)

  # Because the OpenBLAS people feel that life isn't complicated enough, they
  # insist on creating libopenblas.a as a symlink. Here we crudely undo this
  # needlessly overcomplicated operation.
  # ExternalProject_Add_Step(openblas_proj remove_stupid_openblas_symlink
  #   COMMAND ${CMAKE_COMMAND} -E rename libopenblas.a libopenblas.a.symlink
  #   COMMAND ${CMAKE_COMMAND} -E copy libopenblas.a.symlink libopenblas.a
  #   WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/lib
  #   DEPENDEES install)

  add_dependencies(openblas openblas_proj)
endif()
# list(APPEND EXT_INCDIRS ${OPENBLAS_INCLUDE_DIR})
# set(EXT_LIBRARIES openblas;${EXT_LIBRARIES})

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
                               # -DKokkosKernels_ENABLE_TPL_BLAS=ON
                               # -DKokkosKernels_BLAS_LIBRARY_DIRS=${OPENBLAS_LIBRARY_DIR}
                               -DKokkosKernels_ENABLE_TESTS=OFF)
  ExternalProject_Add(kokkoskernels_proj
                      PREFIX ${CMAKE_CURRENT_BINARY_DIR}/kokkos-kernels
                      SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/kokkos-kernels
                      BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/kokkos-kernels
                      INSTALL_DIR ${PROJECT_BINARY_DIR}
                      CMAKE_ARGS ${KOKKOSKERNELS_CMAKE_OPTS}
                      DEPENDS openblas
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

if (${OPENBLAS_LIBRARY_DIR} MATCHES ${PROJECT_BINARY_DIR}) # we built OpenBLAS
  install(FILES ${OPENBLAS_LIBRARY} DESTINATION lib)
endif()
if (${KOKKOSKERNELS_LIBRARY_DIR} MATCHES ${PROJECT_BINARY_DIR}) # we built kokkos kernels
  install(FILES ${KOKKOSKERNELS_LIBRARY} DESTINATION lib)
endif()

