FROM python:3.10.14-slim-bookworm AS builder

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
RUN mkdir -p /sumo 

# Set C++17 as the standard for all builds
ENV CXXFLAGS="-std=c++17"
# Install Arrow and Parquet with C++17 support - FORCED TO VERSION 19
RUN git clone https://github.com/apache/arrow.git /arrow && \
    cd /arrow && \
    git checkout apache-arrow-19.0.0 && \
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
WORKDIR /sumo

# Copy all files to local SUMO directory
COPY . /sumo

# Clean any pre-existing build artifacts that might have been copied
RUN rm -rf build/ CMakeFiles/ CMakeCache.txt cmake_install.cmake Makefile *.cmake

# Set SUMO Parquet configuration
ENV SUMO_PARQUET_ROWGROUP_SIZE=10000

# Build the sumo target using clang
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
      -DCMAKE_INSTALL_PREFIX=/sumo/install \
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

# Stage 2: Runtime-only image (final lightweight image)
FROM python:3.10.14-slim-bookworm AS runtime

# Install ONLY runtime libraries (not -dev packages) + build tools for downstream builds
RUN apt-get update && apt-get install --no-install-recommends -y \
    libxerces-c3.2 libfox-1.6-0 libgdal32 libproj25 \
    libgl2ps1.4 libssl3 libcurl4 libsnappy1v5 \
    zlib1g libre2-9 liblz4-1 libzstd1 libbrotli1 \
    ca-certificates \
    # Add build tools for downstream C++ compilation
    build-essential g++ \
    && rm -rf /var/lib/apt/lists/*

# Copy ONLY the built binaries and libraries
COPY --from=builder /usr/local/lib/ /usr/local/lib/
# Copy header files for development dependencies (Arrow, Parquet, fmt, etc.)
COPY --from=builder /usr/local/include/ /usr/local/include/
COPY --from=builder /sumo/install/ /sumo/
COPY --from=builder /sumo/tools/ /sumo/tools/
COPY --from=builder /sumo/data/ /sumo/data/
# Copy source headers for libsumo development
COPY --from=builder /sumo/src/ /sumo/src/
# These MUST be included in the runtime stage
ENV SUMO_HOME=/sumo
ENV PATH="/sumo/bin:${PATH}"
ENV PYTHONPATH="/sumo/tools:${PYTHONPATH}"
ENV LD_LIBRARY_PATH="/sumo/bin/:/usr/local/lib:${LD_LIBRARY_PATH}"

