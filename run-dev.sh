#!/bin/bash

# Script to run the development environment

echo "Starting development environment..."

# Check if docker-compose.dev.yml exists
if [ ! -f "docker-compose.dev.yml" ]; then
    echo "Error: docker-compose.dev.yml not found!"
    exit 1
fi

# Start the development environment
docker-compose -f docker-compose.dev.yml up

echo "Development environment stopped."