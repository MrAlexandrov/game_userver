#!/bin/bash

# Script to stop the development environment

echo "Stopping development environment..."

# Check if docker-compose.dev.yml exists
if [ ! -f "docker-compose.dev.yml" ]; then
    echo "Error: docker-compose.dev.yml not found!"
    exit 1
fi

# Stop the development environment
docker-compose -f docker-compose.dev.yml down

echo "Development environment stopped."