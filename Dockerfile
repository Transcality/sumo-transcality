FROM python:3.10.14-slim-bookworm

# Install dependencies needed to compile SUMO
RUN apt-get update && apt-get install -y \
    build-essential git cmake python3 clang llvm \
    libxerces-c-dev libfox-1.6-dev libgdal-dev libproj-dev \
    libgl2ps-dev python3-dev swig default-jdk maven libeigen3-dev vim \
    # Dependencies for Arrow and Parquet
    libboost-all-dev libssl-dev libcurl4-openssl-dev \
    rapidjson-dev libgflags-dev libsnappy-dev libz-dev \
    libre2-dev liblz4-dev libzstd-dev libbrotli-dev


# Create directories for both SUMOs
RUN mkdir -p /sumo /sumo-local
WORKDIR /sumo

# Clone official SUMO repository
RUN git clone --recursive https://github.com/eclipse-sumo/sumo.git .

# Build all targets from official repo
RUN mkdir -p build && cd build && \
    cmake \
      -DHAVE_PARQUET=ON \
      -DWITH_PARQUET=ON \
      -DHAVE_S3=ON \
      -DARROW_S3=ON \
      -DHAVE_AZURE=ON \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_CXX_STANDARD_REQUIRED=ON \
      -DCMAKE_INSTALL_PREFIX=/sumo-official/install \
      .. && \
    make -j$(nproc) && \
    make install


# Set C++17 as the standard for all builds
ENV CXXFLAGS="-std=c++17"
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
          -DCMAKE_C_COMPILER=clang \
          -DCMAKE_CXX_COMPILER=clang++ \
          ../cpp && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# Install fmt library with C++17 support
RUN git clone https://github.com/fmtlib/fmt.git /fmt && \
    cd /fmt && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_CXX_STANDARD=17 -DCMAKE_CXX_STANDARD_REQUIRED=ON \
          -DCMAKE_C_COMPILER=clang \
          -DCMAKE_CXX_COMPILER=clang++ \
          .. && \
    make -j$(nproc) && \
    make install 

# Now build only the 'sumo' target from local files
WORKDIR /sumo-local

# Copy all files to local SUMO directory
COPY . /sumo-local/

# Clean any pre-existing build artifacts that might have been copied
RUN rm -rf build/ CMakeFiles/ CMakeCache.txt cmake_install.cmake Makefile *.cmake

# Build the sumo target using clang
RUN mkdir -p build && \
    cmake -B build \
      -DHAVE_PARQUET=ON \
      -DWITH_PARQUET=ON \
      -DHAVE_S3=ON \
      -DARROW_S3=ON \
      -DHAVE_AZURE=ON \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++ \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_CXX_STANDARD_REQUIRED=ON \
      -DCMAKE_INSTALL_PREFIX=/sumo-local/install \
      . && \
    cd build && make -j$(nproc) sumo && \
    cp /sumo-local/bin/sumo /sumo-official/bin/

# Set up final SUMO environment
WORKDIR /sumo-official

# Initialize environment variables
ENV PYTHONPATH="" 
ENV LD_LIBRARY_PATH=""

# Expose environment variables for SUMO
ENV SUMO_HOME=/sumo-official
ENV PATH="/sumo-official/bin:${PATH}"
ENV PYTHONPATH="/sumo-official/tools:${PYTHONPATH}"
ENV LD_LIBRARY_PATH="/sumo-official/bin/:${LD_LIBRARY_PATH}"


