# Use official SUMO image with explicit platform specification
# Note: Official SUMO images are only available for AMD64/x86_64
# On Apple Silicon Macs, this will run under emulation
FROM --platform=linux/amd64 ghcr.io/eclipse-sumo/sumo:main

# Add Apache Arrow/Parquet support
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    wget \
    gnupg \
    lsb-release \
    && wget https://packages.apache.org/artifactory/arrow/$(lsb_release --id --short | tr 'A-Z' 'a-z')/apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb \
    && apt install -y -V ./apache-arrow-apt-source-latest-$(lsb_release --codename --short).deb \
    && apt-get update \
    && apt-get install -y -V libarrow-dev libparquet-dev \
    && rm -rf /var/lib/apt/lists/* ./apache-arrow-apt-source-latest-*.deb 