# Dockerfile.sumo
FROM python:3.10.14-slim-bookworm

# Install dependencies needed to compile SUMO
RUN apt-get update && apt-get install -y \
    build-essential git cmake python3 g++ \
    libxerces-c-dev libfox-1.6-dev libgdal-dev libproj-dev \
    libgl2ps-dev python3-dev swig default-jdk maven libeigen3-dev vim

# Clone SUMO, checkout desired commit
RUN git clone https://github.com/eclipse-sumo/sumo.git /sumo
WORKDIR /sumo
RUN git fetch origin db977ec3e8fab289cd095628a87b0912ff0bedf3
RUN git checkout db977ec3e8fab289cd095628a87b0912ff0bedf3

# Build SUMO
RUN cmake -B build . \
      -DCMAKE_INSTALL_PREFIX=/sumo/build/install 
RUN cmake --build build -j$(nproc)
# Expose environment variables for SUMO
ENV SUMO_HOME=/sumo
ENV PATH="/sumo/bin:$PATH"
ENV PYTHONPATH="/sumo/tools:$PYTHONPATH"
ENV LD_LIBRARY_PATH=L/sumo/bin/:$LD_LIBRARY_PATH
