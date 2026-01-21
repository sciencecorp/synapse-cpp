#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

IMAGE_NAME="synapse-cpp-builder"
CONTAINER_NAME="synapse-cpp-build"

echo "Building Docker image..."
docker build -t "$IMAGE_NAME" .

echo "Build successful!"
echo ""
echo "To run tests:"
echo "  docker run --rm $IMAGE_NAME"
echo ""
echo "To get an interactive shell:"
echo "  docker run --rm -it $IMAGE_NAME /bin/bash"
