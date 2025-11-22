#!/bin/bash

# Script to run tests with coverage
set -e

echo "🧪 Running tests with coverage..."

# Check if coverage tools are available
if ! command -v lcov &> /dev/null; then
    echo "⚠️  lcov not found. Installing..."
    if [[ "$OSTYPE" == "darwin"* ]]; then
        brew install lcov
    else
        echo "Please install lcov manually: sudo apt-get install lcov (Ubuntu/Debian) or equivalent for your system"
        exit 1
    fi
fi

if ! command -v genhtml &> /dev/null; then
    echo "⚠️  genhtml not found. Installing..."
    if [[ "$OSTYPE" == "darwin"* ]]; then
        brew install lcov
    else
        echo "Please install lcov manually: sudo apt-get install lcov (Ubuntu/Debian) or equivalent for your system"
        exit 1
    fi
fi

# Clean previous build
echo "🧹 Cleaning previous build..."
rm -rf build
mkdir -p build
cd build

# Configure with coverage enabled
echo "⚙️  Configuring CMake with coverage enabled..."
cmake -DENABLE_COVERAGE=ON ..

# Build
echo "🔨 Building with coverage..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Run tests
echo "🏃 Running tests..."
./operators_test

# Generate coverage report
echo "📊 Generating coverage report..."
make coverage

echo "✅ Coverage report generated!"
echo "📁 Open coverage/index.html in your browser to view the report"
echo "🧹 Run 'make coverage-clean' to clean coverage files"
