ARG BUILD_TYPE_IN=debug

FROM ubuntu:21.04

# FIXME: read this from command line or set to debug
FROM mjschm/cpppt-ext:$BUILD_TYPE_IN

COPY ./.docker_configs /root
CMD source /root/.bashrc

ENV IN_CONTAINER=true

# Update packages
RUN apt-get update

# Configure time-zone data noninteractively
RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install tzdata

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
  python3 \
  python3-pip \
  ca-certificates && \
  rm -rf /var/lib/apt/lists/*

RUN pip3 install numpy pyyaml sklearn

# We build in /particle
WORKDIR /particle
COPY CMakeLists.txt config.sh ./
COPY ext ext
COPY profiling profiling
COPY src src

ARG BUILD_TYPE_IN=debug
ENV DOCKER_BUILD_TYPE=$BUILD_TYPE_IN
ENV DOCKER_PRECISION=double
ENV DOCKER_SEARCH_TYPE=tree

RUN mkdir build && \
    cd build && \
    ../config.sh && \
    make -j6 && \
    make install


#     cmake \
#         -D CMAKE_INSTALL_PREFIX="./" \
#         -D PARPT_BUILD_TYPE=$DOCKER_BUILD_TYPE \
#         -D CMAKE_VERBOSE_MAKEFILE=ON \
#         -D CMAKE_CXX_COMPILER=g++ \
#         -D CMAKE_C_COMPILER=gcc \
#         -D PARPT_USE_OPENMP=ON \
#         -D PARPT_USE_CUDA=OFF \
#         -D PARPT_PRECISION=$DOCKER_PRECISION \
#         -D PARPT_SEARCH_TYPE=$DOCKER_SEARCH_TYPE \
#         .. && \
#     make -j6 && \
#     make install
