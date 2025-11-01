#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================="
echo "  Hot Reload Test Suite Runner"
echo "========================================="
echo

# Configuration
BUILD_TYPE="${BUILD_TYPE:-Debug}"
RELOADABLE="${RELOADABLE:-ON}"
BUILD_DIR=".build/${BUILD_TYPE}-$([ "$RELOADABLE" = "ON" ] && echo "reloadable" || echo "static")"

# Check for required tools
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: cmake not found${NC}"
    echo "Please install CMake 3.22 or higher"
    exit 1
fi

if ! command -v ninja &> /dev/null; then
    echo -e "${YELLOW}Warning: ninja not found, falling back to default generator${NC}"
    GENERATOR=""
else
    GENERATOR="-GNinja"
fi

# Create build directory
echo "Creating build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Configure CMake
echo
echo "Configuring CMake..."
cmake $GENERATOR \
    -S . \
    -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DRELOADABLE="$RELOADABLE" \
    -DBUILD_TESTS=ON

if [ $? -ne 0 ]; then
    echo -e "${RED}Error: CMake configuration failed${NC}"
    echo
    echo "Common issues:"
    echo "  - Missing OpenGL development libraries"
    echo "  - Missing SDL3 dependencies"
    echo
    echo "Try installing dependencies:"
    echo "  Ubuntu/Debian: sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev"
    echo "  Fedora: sudo dnf install mesa-libGL-devel mesa-libGLU-devel"
    exit 1
fi

# Build
echo
echo "Building project..."
cmake --build "$BUILD_DIR" --parallel

if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Build failed${NC}"
    exit 1
fi

# Run tests
echo
echo "Running tests..."
echo
"$BUILD_DIR/hot_reload_tests"

TEST_RESULT=$?

echo
if [ $TEST_RESULT -eq 0 ]; then
    echo -e "${GREEN}✅ All tests passed!${NC}"
else
    echo -e "${RED}❌ Some tests failed${NC}"
fi

exit $TEST_RESULT
