FROM python:3.10.14-slim-bookworm AS builder

# Install dependencies needed to compile SUMO
RUN apt-get update && apt-get install -y \
    build-essential git cmake python3 gcc g++ llvm \
    libxerces-c-dev libfox-1.6-dev libgdal-dev libproj-dev \
    libgl2ps-dev python3-dev swig default-jdk maven libeigen3-dev vim \
    # Dependencies for Arrow and Parquet
    libboost-all-dev libssl-dev libcurl4-openssl-dev \
    rapidjson-dev libgflags-dev libsnappy-dev libz-dev \
    libre2-dev liblz4-dev libzstd-dev libbrotli-dev


# Create directories for both SUMOs
RUN mkdir -p /sumo 

# Set C++17 as the standard for all builds
ENV CXXFLAGS="-std=c++17"
# Install Arrow and Parquet with C++17 support - FORCED TO VERSION 19
RUN git clone https://github.com/apache/arrow.git /arrow && \
    cd /arrow && \
    git checkout apache-arrow-20.0.0 && \
    mkdir build && \
    cd build && \
    cmake -DARROW_S3=ON -DARROW_PARQUET=ON -DARROW_DATASET=ON -DARROW_WITH_SNAPPY=ON \
          -DARROW_WITH_ZLIB=ON -DARROW_WITH_ZSTD=ON -DARROW_WITH_BROTLI=ON -DARROW_WITH_LZ4=ON \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_CXX_STANDARD=17 \
          -DCMAKE_CXX_STANDARD_REQUIRED=ON \
          -DCMAKE_C_COMPILER=gcc \
          -DCMAKE_CXX_COMPILER=g++ \
          -DARROW_BUILD_SHARED=ON \
          -DARROW_BUILD_STATIC=OFF \
          -DARROW_PARQUET_SHARED=ON \
          ../cpp && \
    make -j$(nproc) && \
    make install

# Install fmt library with C++17 support
RUN git clone https://github.com/fmtlib/fmt.git /fmt && \
    cd /fmt && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON \
          -DCMAKE_C_COMPILER=gcc \
          -DCMAKE_CXX_COMPILER=g++ \
          .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig 

# Now build only the 'sumo' target from local files
WORKDIR /sumo

# Copy all files to local SUMO directory
COPY . /sumo

# Clean any pre-existing build artifacts that might have been copied
RUN rm -rf build/ CMakeFiles/ CMakeCache.txt cmake_install.cmake Makefile *.cmake

# Set SUMO Parquet configuration
ENV SUMO_PARQUET_ROWGROUP_SIZE=10000

# Build the sumo target using gcc
RUN mkdir -p build && cd build && \
    cmake \
        -DHAVE_PARQUET=ON \
      -DWITH_PARQUET=ON \
      -DHAVE_S3=ON \
      -DARROW_S3=ON \
      -DHAVE_AZURE=ON \
      -DCMAKE_C_COMPILER=gcc \
      -DCMAKE_CXX_COMPILER=g++ \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_CXX_STANDARD_REQUIRED=ON \
      -DCMAKE_INSTALL_PREFIX=/sumo/install \
      -DArrow_DIR=/usr/local/lib/cmake/arrow \
      -DParquet_DIR=/usr/local/lib/cmake/arrow \
      -DCMAKE_CXX_FLAGS="-I/usr/local/include" \
      -DCMAKE_EXE_LINKER_FLAGS="-L/usr/local/lib -larrow -lparquet" \
      .. && \
      make -j$(nproc) && \
      make install

# Set up final SUMO environment
WORKDIR /sumo

# Initialize environment variables
ENV PYTHONPATH="" 
ENV LD_LIBRARY_PATH=""

# Expose environment variables for SUMO
ENV SUMO_HOME=/sumo
ENV PATH="/sumo/bin:${PATH}"
ENV PYTHONPATH="/sumo/tools:${PYTHONPATH}"
ENV LD_LIBRARY_PATH="/sumo/bin/:${LD_LIBRARY_PATH}"

