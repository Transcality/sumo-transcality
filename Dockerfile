FROM python:3.10.14-slim-bookworm

ENV DEBIAN_FRONTEND=noninteractive
ENV SUMO_HOME=/sumo
ENV PATH="${SUMO_HOME}/bin:${PATH}"
ENV PYTHONPATH="${SUMO_HOME}/tools:${PYTHONPATH:-}"
# Help CMake find Debian-provided package configs
ENV CMAKE_PREFIX_PATH="/usr/lib/x86_64-linux-gnu/cmake:${CMAKE_PREFIX_PATH:-}"
# Ensure C++17
ENV CXXFLAGS="-std=c++17"

# Build & dev deps + Apache Arrow APT repo (Bookworm)
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates wget gnupg lsb-release \
    build-essential git cmake ninja-build pkg-config \
    swig default-jdk maven python3-dev \
    # SUMO deps
    libxerces-c-dev libfox-1.6-dev libgdal-dev libproj-dev libgl2ps-dev freeglut3-dev libeigen3-dev \
    # Optional but useful
    libfmt-dev libgoogle-perftools-dev \
    # Arrow runtime/codec deps
    libssl-dev libcurl4-openssl-dev rapidjson-dev libgflags-dev \
    libsnappy-dev zlib1g-dev liblz4-dev libzstd-dev libbrotli-dev \
 && wget -q "https://packages.apache.org/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb" \
 && apt-get install -y -V ./apache-arrow-apt-source-latest-*.deb \
 && rm -f ./apache-arrow-apt-source-latest-*.deb \
 && apt-get update && apt-get install -y --no-install-recommends \
    libarrow-dev libparquet-dev libarrow-dataset-dev \
 && rm -rf /var/lib/apt/lists/*

# Use local SUMO source tree
WORKDIR /sumo
COPY . /sumo/

# Configure, build, install (keep prefix at /sumo)
RUN cmake -S . -B build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON \
      -DCMAKE_INSTALL_PREFIX=/sumo \
      -DWITH_PARQUET=ON \
      -DWITH_FOX=ON \
 && cmake --build build -j"$(nproc)" \
 && cmake --install build \
 && strip /sumo/bin/* 2>/dev/null || true

# Useful at runtime if any libs are under /sumo/lib
ENV LD_LIBRARY_PATH="/sumo/lib:${LD_LIBRARY_PATH:-}"
