# Builder stage
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install build deps in single layer
RUN apt-get update && apt-get install -y --no-install-recommends \
    wget lsb-release gnupg ca-certificates git cmake g++ \
    astyle ccache debhelper devscripts default-jdk doxygen freeglut3-dev \
    gettext graphviz help2man hugo libeigen3-dev libfmt-dev libfox-1.6-dev \
    libgdal-dev libgeos-dev libgl2ps-dev libgoogle-perftools-dev libgtest-dev \
    libproj-dev libv8-dev libxerces-c-dev libxrandr-dev maven mkdocs mono-mcs \
    pipx plantuml pre-commit python-is-python3 python3-build python3-dev \
    python3-pip python3-setuptools swig xvfb \
    && wget -q https://packages.apache.org/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb \
    && apt-get install -y -V ./apache-arrow-apt-source-latest-*.deb \
    && apt-get update && apt-get install -y --no-install-recommends libarrow-dev libparquet-dev \
    && rm -rf /var/lib/apt/lists/* ./apache-arrow-apt-source-latest-*.deb

# Build SUMO
WORKDIR /usr/src

ARG SUMO_REPO=https://github.com/Transcality/sumo-transcality
ARG SUMO_BRANCH=simreply-fix

RUN git clone --recursive --depth=1 --branch $SUMO_BRANCH $SUMO_REPO sumo\
    && cd sumo \
    && cmake -B build -DCMAKE_BUILD_TYPE=Release . \
    && cmake --build build -j$(nproc) \
    && strip build/bin/* 2>/dev/null || true

# Runtime stage
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV SUMO_HOME="/usr/sumo"
ENV PATH="${SUMO_HOME}/bin:${PATH}"

# Install minimal runtime deps with Apache Arrow + development tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    wget lsb-release gnupg ca-certificates g++ cmake \
    && wget -q https://packages.apache.org/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb \
    && apt-get install -y -V ./apache-arrow-apt-source-latest-*.deb \
    && apt-get update && apt-get install -y --no-install-recommends \
    libfox-1.6-0 libgdal30 libgeos-c1v5 libgl2ps1.4 libproj22 \
    libxerces-c3.2 python3-minimal libfmt8 freeglut3 \
    libgl1-mesa-glx libglu1-mesa libgoogle-perftools4 \
    libarrow-dev libparquet-dev \
    && apt-get install -y --no-install-recommends libarrow-dev libparquet-dev || true \
    && rm -rf /var/lib/apt/lists/* ./apache-arrow-apt-source-latest-*.deb

# Copy built SUMO with development files
COPY --from=builder /usr/src/sumo/bin ${SUMO_HOME}/bin
COPY --from=builder /usr/src/sumo/data ${SUMO_HOME}/data
COPY --from=builder /usr/src/sumo/tools ${SUMO_HOME}/tools
COPY --from=builder /usr/src/sumo/src ${SUMO_HOME}/src


WORKDIR ${SUMO_HOME}

