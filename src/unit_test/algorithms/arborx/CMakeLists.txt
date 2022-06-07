# we need stuff from the global include dirs
include_directories(${PARPT_INCDIRS})

add_executable(AX_test arborx_test.cpp)
add_executable(AX_compare ax_brute_compare.cpp)
# add the tests to ctest
# NOTE: requires a little extra work since the tests are run by bash scripts
  # and involve python scripts
find_program(BASH_PROGRAM bash)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_ax_compare.sh.in
               ${CMAKE_CURRENT_BINARY_DIR}/run_ax_compare.sh @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_ax_test.sh.in
               ${CMAKE_CURRENT_BINARY_DIR}/run_ax_test.sh @ONLY)
add_test(AX_smoke_test ${BASH_PROGRAM}
         ${CMAKE_CURRENT_BINARY_DIR}/run_ax_test.sh)
add_test(AX_compare_test ${BASH_PROGRAM}
         ${CMAKE_CURRENT_BINARY_DIR}/run_ax_compare.sh)
# copy th required python scripts over
file(COPY verify_arborx.py3 gen_pts.py3 DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# FIXME: this seems not to be necessary but keeping around in case the
# linux build goes bad again
# add arborx to the list of libraries that this project needs
# set(AX_LIBS ${LIBRARIES} arborx)
add_dependencies(Utils yaml_cpp kokkos kokkoskernels arborx spdlog)

if(PARPT_USE_CUDA)
  # find_package(CUDA)
  cuda_add_cublas_to_target(AX_compare)
  # list(APPEND LIBRARIES; ${CUDA_cusparse_LIBRARY})
endif()

target_link_libraries(AX_test
                      ${LIBRARIES})
target_link_libraries(AX_compare
                      ${LIBRARIES})

# FIXME: can I get rid of this?
# if (LINUX)
#   set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
# endif()

# add the include directories
target_include_directories(AX_test PUBLIC
                           ${PARPT_INCDIRS})
target_include_directories(AX_compare PUBLIC
                           ${PARPT_INCDIRS})

# install the executable and the unit test utility scripts
# set(AX_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/unit_test/arborx)
# install(TARGETS AX_test DESTINATION ${AX_INSTALL_DIR})
# install(TARGETS AX_compare DESTINATION ${AX_INSTALL_DIR})
# install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_ax_test.sh
#                  ${CMAKE_CURRENT_BINARY_DIR}/run_ax_compare.sh
#                  DESTINATION ${AX_INSTALL_DIR})
