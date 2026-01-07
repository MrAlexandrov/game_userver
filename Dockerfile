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

# Create user with specific UID to match host (will be overridden by run_as_user.sh if needed)
ARG USER_UID=1000
ARG USER_GID=1000

# Only create user if it doesn't exist (avoid UID conflict)
RUN if ! id -u ${USER_UID} > /dev/null 2>&1; then \
        groupadd -g ${USER_GID} user && \
        useradd -m -u ${USER_UID} -g ${USER_GID} -s /bin/bash user; \
    fi

# Set working directory
WORKDIR /game_userver

# Copy project files
COPY --chown=user:user . /game_userver

# Switch to user
USER user

# Set environment variables for build
ENV PREFIX=/home/user/.local
ENV CCACHE_DIR=/game_userver/.ccache
ENV CORES_DIR=/cores

# Expose port
EXPOSE 8080

# Default command (will be overridden by docker-compose)
CMD ["bash"]