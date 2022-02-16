include_directories(${PARPT_INCDIRS})
# add utilities.cpp as a library
add_executable(AX_test arborx_test.cpp)
set(AX_LIBS yaml_cpp kokkos kokkoskernels arborx)
add_dependencies(${AX_LIBS})

target_link_libraries(AX_test
                      ${AX_LIBS}
                      )

# add the include directories
target_include_directories(AX_test PUBLIC
                           ${PARPT_INCDIRS}
                          )

# FIXME: this is where it should go
set(AX_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/unit_test/arborx)
install(TARGETS AX_test DESTINATION ${AX_INSTALL_DIR})
install(PROGRAMS run_ax_test.sh verify_arborx.py3 gen_pts.py3 DESTINATION ${AX_INSTALL_DIR})
# this one's for easy testing
# install(TARGETS AX_test DESTINATION ${CMAKE_INSTALL_PREFIX})
