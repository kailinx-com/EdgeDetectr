#!/bin/bash

# Quick coverage check script
set -e

echo "🔍 Quick coverage check..."

# Check if we're in a build directory with coverage
if [ ! -f "CMakeCache.txt" ] || ! grep -q "ENABLE_COVERAGE:BOOL=ON" CMakeCache.txt; then
    echo "⚠️  Coverage not enabled. Run from a build directory with -DENABLE_COVERAGE=ON"
    exit 1
fi

# Run tests if not already run
if [ ! -f "coverage.info" ]; then
    echo "🏃 Running tests to generate coverage data..."
    ./operators_test
fi

# Generate simple coverage report
echo "📊 Generating coverage report..."
make coverage

echo "✅ Coverage report ready at coverage/index.html"
