FROM python:3.10.14-slim-bookworm

# Set C++17 as the standard for all builds
ENV CXXFLAGS="-std=c++17"

# Install dependencies needed to compile SUMO
RUN apt-get update && apt-get install -y \
    build-essential git cmake python3 g++ \
    libxerces-c-dev libfox-1.6-dev libgdal-dev libproj-dev \
    libgl2ps-dev python3-dev swig default-jdk maven libeigen3-dev vim \
    # Dependencies for Arrow and Parquet
    libboost-all-dev libssl-dev libcurl4-openssl-dev \
    rapidjson-dev libgflags-dev libsnappy-dev libz-dev \
    libre2-dev liblz4-dev libzstd-dev libbrotli-dev

# Install Arrow and Parquet with C++17 support
RUN git clone https://github.com/apache/arrow.git /arrow && \
    cd /arrow && \
    mkdir build && \
    cd build && \
    cmake -DARROW_S3=ON -DARROW_PARQUET=ON -DARROW_DATASET=ON -DARROW_WITH_SNAPPY=ON \
          -DARROW_WITH_ZLIB=ON -DARROW_WITH_ZSTD=ON -DARROW_WITH_BROTLI=ON -DARROW_WITH_LZ4=ON \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_CXX_STANDARD=17 \
          -DCMAKE_CXX_STANDARD_REQUIRED=ON \
          ../cpp && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# Install fmt library with C++17 support
RUN git clone https://github.com/fmtlib/fmt.git /fmt && \
    cd /fmt && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON .. && \
    make -j$(nproc) && \
    make install

# Create directory for SUMO
RUN mkdir -p /sumo
WORKDIR /sumo

# Copy local SUMO files instead of cloning
COPY . /sumo/

# Build SUMO with Parquet, S3 and Azure support and C++17
RUN mkdir -p build && cd build && \
    cmake \
      -DHAVE_PARQUET=ON \
      -DWITH_PARQUET=ON \
      -DHAVE_S3=ON \
      -DARROW_S3=ON \
      -DHAVE_AZURE=ON \
      -DCMAKE_C_COMPILER=/usr/bin/gcc \
      -DCMAKE_CXX_COMPILER=/usr/bin/g++ \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_CXX_STANDARD_REQUIRED=ON \
      -DCMAKE_INSTALL_PREFIX=/sumo/build/install \
      .. && \
    make -j$(nproc) sumo

# Expose environment variables for SUMO
ENV SUMO_HOME=/sumo
ENV PATH="/sumo/bin:$PATH"
ENV PYTHONPATH="/sumo/tools:$PYTHONPATH"
ENV LD_LIBRARY_PATH="/sumo/bin/:$LD_LIBRARY_PATH"
