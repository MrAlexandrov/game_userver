# Multi-stage build for game_userver service
FROM ghcr.io/userver-framework/ubuntu-24.04-userver:latest AS builder

# Install additional dependencies if needed
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    postgresql-client \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Build the project in release mode
RUN cmake --preset release && \
    cmake --build build-release -j $(nproc) --target game_userver

# Production stage
FROM ubuntu:24.04

# Install runtime dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    ca-certificates \
    libpq5 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN groupadd -r appuser && useradd -r -g appuser appuser

# Set working directory
WORKDIR /app

# Copy built binary and configs from builder
COPY --from=builder /app/build-release/game_userver /app/game_userver
COPY --from=builder /app/configs /app/configs

# Change ownership to non-root user
RUN chown -R appuser:appuser /app

# Switch to non-root user
USER appuser

# Expose ports
# HTTP server port
EXPOSE 8080
# gRPC server port
EXPOSE 8081

# Run the service
CMD ["/app/game_userver", "--config", "/app/configs/static_config.yaml", "--config_vars", "/app/configs/config_vars.yaml"]
