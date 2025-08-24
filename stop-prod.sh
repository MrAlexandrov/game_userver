#!/bin/bash

# Script to stop the production environment

echo "Stopping production environment..."

# Check if docker-compose.run.yml exists
if [ ! -f "docker-compose.run.yml" ]; then
    echo "Error: docker-compose.run.yml not found!"
    exit 1
fi

# Stop the production environment
docker-compose -f docker-compose.run.yml down

echo "Production environment stopped."