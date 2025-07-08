# OpenFOAM MCP Server Docker Image
FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    curl \
    wget \
    python3 \
    python3-pip \
    nlohmann-json3-dev \
    libboost-all-dev \
    libsqlite3-dev \
    software-properties-common \
    gnupg \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Install OpenFOAM 12
RUN wget -O - https://dl.openfoam.org/gpg.key | apt-key add - && \
    add-apt-repository http://dl.openfoam.org/ubuntu && \
    apt-get update && \
    apt-get install -y openfoam12 && \
    rm -rf /var/lib/apt/lists/*

# Set up OpenFOAM environment
ENV FOAM_VERSION=12
ENV FOAM_INST_DIR=/opt/openfoam12
ENV WM_PROJECT_DIR=/opt/openfoam12

# Create app directory
WORKDIR /app

# Copy source code
COPY src/ ./src/
COPY CMakeLists.txt ./
COPY README.md ./

# Source OpenFOAM environment and build
RUN /bin/bash -c "source /opt/openfoam12/etc/bashrc && \
    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20 && \
    cmake --build build --parallel $(nproc)"

# Create non-root user
RUN useradd -m -u 1000 openfoam && \
    chown -R openfoam:openfoam /app

USER openfoam

# Set up runtime environment
ENV PATH="/app/build:$PATH"

# Health check
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD /app/build/openfoam-mcp-server --version || exit 1

# Default command
CMD ["/app/build/openfoam-mcp-server"]