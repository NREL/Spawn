# This is a minimal environment that can build Spawn

FROM ubuntu:20.04

RUN apt update -y && \
DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt install -y \
clang-10 \
libclang-10-dev \
llvm-10-dev \
liblld-10-dev \
curl \
gfortran \
build-essential \
python3-pip \
autotools-dev \
git \
ccache \
libblas-dev \
liblapack-dev \
autoconf \
default-jre-headless \
libcurl4-openssl-dev \
uuid-dev \
pkg-config \
libboost-all-dev

RUN curl -LO https://github.com/Kitware/CMake/releases/download/v3.28.1/cmake-3.28.1-linux-x86_64.tar.gz && \
tar -C /usr/local --strip-components=1 -xzf cmake-3.28.1-linux-x86_64.tar.gz

RUN pip3 install conan==1.62
