FROM ghcr.io/userver-framework/ubuntu-24.04-userver-base:latest

# Install C++23 compatible compilers (already available in Ubuntu 24.04)
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    g++-13 \
    gcc-13 \
    cmake \
    ccache \
    && rm -rf /var/lib/apt/lists/*

# Set C++23 compilers as default
ENV CC=gcc-13
ENV CXX=g++-13

# Set working directory
WORKDIR /game_userver

# Set environment variables for build
ENV CCACHE_DIR=/game_userver/.ccache
ENV CORES_DIR=/cores

# Expose port
EXPOSE 8080

# Default command - run as root, user creation handled by run_as_user.sh
CMD ["bash"]