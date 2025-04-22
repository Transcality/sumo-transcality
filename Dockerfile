
FROM gcc:16 AS builder

# 1) Install build-time dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      build-essential cmake git python3.10 python3.10-dev python3-pip swig \
      default-jdk maven \
      libxerces-c-dev libfox-1.6-dev libgdal-dev libproj-dev libgl2ps-dev \
      libboost-all-dev libssl-dev libcurl4-openssl-dev \
      rapidjson-dev libgflags-dev libsnappy-dev zlib1g-dev liblz4-dev libzstd-dev libbrotli-dev libre2-dev \
    && rm -rf /var/lib/apt/lists/*

# 2) Build and install Apache Arrow C++ (pin to a stable tag)
ARG ARROW_VERSION=15.0.0
RUN git clone --branch apache-arrow-${ARROW_VERSION} --depth 1 \
        https://github.com/apache/arrow.git /arrow && \
    mkdir -p /arrow/cpp/build && cd /arrow/cpp/build && \
    cmake -DARROW_PARQUET=ON -DARROW_DATASET=ON -DARROW_S3=ON \
          -DARROW_WITH_SNAPPY=ON -DARROW_WITH_ZLIB=ON \
          -DARROW_WITH_ZSTD=ON    -DARROW_WITH_BROTLI=ON -DARROW_WITH_LZ4=ON \
          -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc) && make install && ldconfig && \
    rm -rf /arrow

# 3) Build and install fmt (pin to a stable tag)
ARG FMT_VERSION=10.1.0
RUN git clone --branch ${FMT_VERSION} --depth 1 \
        https://github.com/fmtlib/fmt.git /fmt && \
    mkdir /fmt/build && cd /fmt/build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc) && make install && \
    rm -rf /fmt

# 4) Build and install SUMO itself
WORKDIR /sumo
COPY . /sumo
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DWITH_PARQUET=ON -DHAVE_PARQUET=ON \
          -DWITH_S3=ON -DARROW_S3=ON \
          -DWITH_AZURE=ON \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          .. && \
    make -j$(nproc) sumo && make install

#
# ─── STAGE 2: RUNTIME IMAGE ───────────────────────────────────────────────────
#
FROM python:3.10.14-slim-bookworm

# 5) Only the dynamic libraries SUMO needs at runtime
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      libxerces-c3 libfox-1.6-0v5 libgdal-dev libproj-dev libgl2ps0 \
      libboost-all-dev libssl-dev libcurl4-openssl-dev \
      rapidjson-dev libgflags-dev libsnappy-dev zlib1g liblz4-1 libzstd1 libbrotli1 libre2-dev \
    && rm -rf /var/lib/apt/lists/*

# 6) Copy in all of Arrow, fmt and SUMO from the builder
COPY --from=builder /usr/local /usr/local

# 7) Environment
ENV SUMO_HOME=/usr/local
ENV PATH="$SUMO_HOME/bin:$PATH"
ENV PYTHONPATH="$SUMO_HOME/tools:$PYTHONPATH"
ENV LD_LIBRARY_PATH="$SUMO_HOME/bin:$LD_LIBRARY_PATH"

WORKDIR /sumo
ENTRYPOINT ["sumo"]
CMD ["--help"]
