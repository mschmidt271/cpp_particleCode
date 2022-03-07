# we need stuff from the global include dirs
include_directories(${PARPT_INCDIRS})
add_executable(AX_test arborx_test.cpp)
# FIXME: this seems not to be necessary but keeping around in case the
# linux build goes bad again
# add arborx to the list of libraries that this project needs
# set(AX_LIBS ${LIBRARIES} arborx)

target_link_libraries(AX_test
                      ${LIBRARIES}
                      )

# FIXME: can I get rid of this?
# if (LINUX)
#   set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
# endif()

# add the include directories
target_include_directories(AX_test PUBLIC
                           ${PARPT_INCDIRS}
                          )

# install the executable and the unit test utility scripts
set(AX_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/unit_test/arborx)
install(TARGETS AX_test DESTINATION ${AX_INSTALL_DIR})
install(PROGRAMS run_ax_test.sh verify_arborx.py3 gen_pts.py3 DESTINATION ${AX_INSTALL_DIR})
