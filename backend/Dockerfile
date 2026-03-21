# Stage 1: Build
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-13 \
    g++-13 \
    cmake \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    zlib1g-dev \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100 \
    && rm -rf /var/lib/apt/lists/*

# Install vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg \
    && /opt/vcpkg/bootstrap-vcpkg.sh

ENV VCPKG_ROOT=/opt/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

WORKDIR /app

# Copy dependency manifest first for layer caching
# Build context is repo root, so paths are relative to that
COPY backend/vcpkg.json ./backend/
WORKDIR /app/backend
RUN ARCH=$(uname -m) && \
    if [ "$ARCH" = "x86_64" ]; then TRIPLET="x64-linux"; \
    elif [ "$ARCH" = "aarch64" ]; then TRIPLET="arm64-linux"; \
    else TRIPLET="x64-linux"; fi && \
    vcpkg install --triplet $TRIPLET

# Copy proto and backend source
WORKDIR /app
COPY proto/ ./proto/
COPY backend/ ./backend/

WORKDIR /app/backend
RUN ARCH=$(uname -m) && \
    if [ "$ARCH" = "x86_64" ]; then TRIPLET="x64-linux"; \
    elif [ "$ARCH" = "aarch64" ]; then TRIPLET="arm64-linux"; \
    else TRIPLET="x64-linux"; fi && \
    mkdir -p build && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
        -DVCPKG_TARGET_TRIPLET=$TRIPLET \
    && make -j$(nproc)

# Stage 2: Runtime
FROM ubuntu:24.04 AS runtime

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/backend/build/teleop-server .

RUN mkdir -p /var/log/teleop

EXPOSE 9001

ENTRYPOINT ["./teleop-server"]
CMD ["--config", "/app/config/default.json"]
