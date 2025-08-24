#!/bin/bash

# Script to run the production environment

echo "Starting production environment..."

# Check if docker-compose.run.yml exists
if [ ! -f "docker-compose.run.yml" ]; then
    echo "Error: docker-compose.run.yml not found!"
    exit 1
fi

# Start the production environment
docker-compose -f docker-compose.run.yml up

echo "Production environment stopped."