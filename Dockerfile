FROM ubuntu:21.04

# Process build-time arguments.
ARG BUILD_TYPE=Debug
ARG PRECISION=double
ARG PACK_SIZE=1

# Update packages
RUN apt-get update

# Get dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
  cmake \
  gcc \
  g++ \
  git \
  make \
  vim \
  pkg-config \
  zlib1g-dev \
  ca-certificates && \
  rm -rf /var/lib/apt/lists/*

# We build in /particle
WORKDIR /particle
COPY CMakeLists.txt config.sh ./
COPY ext ext
COPY profiling profiling
COPY src src

# RUN mkdir build && \
#   cd build && \
#   cmake \
#         -D CMAKE_INSTALL_PREFIX="./" \
#         -D PARPT_BUILD_TYPE="debug" \
#         -D CMAKE_VERBOSE_MAKEFILE=ON \
#         -D CMAKE_CXX_COMPILER=g++ \
#         -D CMAKE_C_COMPILER=gcc \
#         -D PARPT_USE_OPENMP=true \
#         -D PARPT_USE_CUDA=false \
#         -D PARPT_PRECISION="double" \
#         -D PARPT_SEARCH_TYPE="tree" \
#         .. && \
#   make -j6 && \
#   make install
