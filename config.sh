#!/bin/bash

rm -rf CMake*

cmake .. \
    -D CMAKE_INSTALL_PREFIX="./install"
