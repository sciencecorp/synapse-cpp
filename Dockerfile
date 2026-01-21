FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    ninja-build \
    autoconf \
    automake \
    libtool \
    python3 \
    wget \
    gpg \
    && rm -rf /var/lib/apt/lists/*

# Install CMake 3.27 via direct binary download (compatible with preset v6 and older vcpkg)
ARG CMAKE_VERSION=3.27.9
RUN wget -qO- "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-aarch64.tar.gz" \
    | tar --strip-components=1 -xz -C /usr/local

# Set up vcpkg (VCPKG_FORCE_SYSTEM_BINARIES required for ARM platforms)
ENV VCPKG_ROOT=/vcpkg
ENV VCPKG_FORCE_SYSTEM_BINARIES=1
RUN git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT && \
    cd $VCPKG_ROOT && \
    git checkout 1751f9f8c732c2e6f9e81ce56c10e4c4aa265b4a && \
    ./bootstrap-vcpkg.sh

WORKDIR /src

# Copy source files (excluding git dirs and build artifacts via .dockerignore)
COPY . .

# Clone submodules at pinned versions
RUN rm -rf external/sciencecorp/synapse-api && \
    git clone --branch v2.1.0 https://github.com/sciencecorp/synapse-api.git external/sciencecorp/synapse-api

RUN if [ ! -d "external/sciencecorp/vcpkg/ports" ]; then \
        rm -rf external/sciencecorp/vcpkg && \
        git clone https://github.com/sciencecorp/vcpkg.git external/sciencecorp/vcpkg; \
    fi

# Configure and build
RUN cmake --preset=static -DCMAKE_BUILD_TYPE=Debug -DVCPKG_MANIFEST_FEATURES="examples;tests"
RUN cmake --build build

# Run tests
CMD ["./build/synapse_tests"]
