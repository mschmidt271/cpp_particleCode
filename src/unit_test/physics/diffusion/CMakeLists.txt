# we need stuff from the global include dirs
include_directories(${PARPT_INCDIRS})

# add the tests to ctest
# NOTE: requires a little extra work since the tests are run by bash scripts
  # and involve python scripts
find_program(BASH_PROGRAM bash)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run_mt_test.sh.in
               ${CMAKE_CURRENT_BINARY_DIR}/run_mt_test.sh @ONLY)
add_test(MT_test ${BASH_PROGRAM}
         ${CMAKE_CURRENT_BINARY_DIR}/run_mt_test.sh)
# copy the data and utils dirs
file(COPY utils data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_dependencies(MT_test parPT)

# install the executable and the unit test utility scripts
set(MT_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/unit_test/physics/diffusion)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/run_MT_test.sh
                 DESTINATION ${MT_INSTALL_DIR})
