# this is the default value, though either choice can be made at build-time
# via the command line argument --build-arg=[debug, release]
ARG BUILD_TYPE_IN=debug

FROM ubuntu:21.04

# this contains all the pre-built TPLs
FROM mjschm/ko_pt:$BUILD_TYPE_IN

# copy in some config files to make the environment more user-friendly (for Mike)
COPY ./.docker_configs /root
CMD source /root/.bashrc

# set this env variable so cmake knows it's in a container
ENV IN_CONTAINER=true

# configure time-zone data noninteractively
RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y install tzdata

# get system-level dependencies
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

# install the python dependencies, mainly used for testing
RUN pip3 install numpy pyyaml sklearn

# unset proxy variables that are user-specific
ENV no_proxy=
ENV https_proxy=
ENV NO_PROXY=
ENV HTTPS_PROXY=
ENV HTTP_PROXY=
ENV http_proxy=

# remove user-specific config files
RUN cd /root && rm -rf .wgetrc .env .pip .sandia

# We build in /particle
WORKDIR /particle
# copy in the source files
COPY CMakeLists.txt config.sh ./
COPY ext ext
COPY profiling profiling
COPY src src

# same deal as above, except we have to do it again because ARGs get destroyed
# after FROM statements
ARG BUILD_TYPE_IN=debug
# set env variables to be used by the cmake configure step
ENV DOCKER_BUILD_TYPE=$BUILD_TYPE_IN
ENV DOCKER_PRECISION=double
ENV DOCKER_SEARCH_TYPE=tree

# build the dang thing
RUN mkdir build && \
    cd build && \
    ../config.sh && \
    make -j6 && \
    make install
