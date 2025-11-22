# Test Coverage Guide

This guide explains how to measure and analyze test coverage for the EdgeDetectr operators project.

## What is Test Coverage?

Test coverage measures how much of your source code is executed during testing. It helps identify:
- Untested code paths
- Dead code
- Areas that need more testing
- Overall test quality

## Prerequisites

### Required Tools

1. **GCC or Clang** (already available on your system)
2. **lcov** - for collecting coverage data
3. **genhtml** - for generating HTML reports

### Install Coverage Tools

On macOS:
```bash
brew install lcov
```

On Ubuntu/Debian:
```bash
sudo apt-get install lcov
```

On CentOS/RHEL:
```bash
sudo yum install lcov
```

## Running Tests with Coverage

### Option 1: Use the Coverage Script (Recommended)

```bash
# Run full coverage build and test
./run-coverage.sh
```

This script will:
1. Install required tools if missing
2. Clean and rebuild with coverage enabled
3. Run all tests
4. Generate HTML coverage report

### Option 2: Manual CMake Configuration

```bash
# Create build directory
mkdir -p build && cd build

# Configure with coverage enabled
cmake -DENABLE_COVERAGE=ON ..

# Build
make -j$(nproc)

# Run tests
./operators_test

# Generate coverage report
make coverage
```

### Option 3: Quick Coverage Check

If you already have a coverage-enabled build:
```bash
cd build
./quick-coverage.sh
```

## Understanding Coverage Reports

### HTML Report

After running coverage, open `build/coverage/index.html` in your browser to see:
- Overall coverage percentage
- File-by-file breakdown
- Line-by-line coverage details
- Uncovered code highlighted in red

### Coverage Metrics

- **Line Coverage**: Percentage of lines executed
- **Branch Coverage**: Percentage of conditional branches taken
- **Function Coverage**: Percentage of functions called

## Coverage Targets

### Available Make Targets

- `make coverage` - Generate full HTML coverage report
- `make coverage-clean` - Remove coverage files and reports

### CMake Options

- `-DENABLE_COVERAGE=ON` - Enable coverage instrumentation
- `-DENABLE_COVERAGE=OFF` - Disable coverage (default)

## Interpreting Results

### Good Coverage Indicators

- **80%+ line coverage** - Good test coverage
- **90%+ line coverage** - Excellent test coverage
- **100% line coverage** - Complete coverage (rare and often unnecessary)

### Areas to Focus On

1. **Critical business logic** - Should have high coverage
2. **Error handling paths** - Often overlooked
3. **Edge cases** - Boundary conditions
4. **New features** - Ensure they're tested

### Common Coverage Issues

1. **Dead code** - Code that's never executed
2. **Platform-specific code** - May not run on your system
3. **Error handling** - Exception paths may be hard to trigger
4. **Third-party code** - External libraries shouldn't affect your coverage

## Best Practices

### Writing Testable Code

1. **Keep functions small** - Easier to test thoroughly
2. **Minimize side effects** - Pure functions are easier to test
3. **Use dependency injection** - Mock external dependencies
4. **Separate concerns** - Business logic separate from I/O

### Improving Coverage

1. **Add missing test cases** for uncovered code paths
2. **Test error conditions** and edge cases
3. **Use parameterized tests** for multiple scenarios
4. **Mock external dependencies** to test error handling

### Coverage Goals

- **Minimum**: 70% line coverage
- **Target**: 80-90% line coverage
- **Stretch**: 95%+ line coverage

## Troubleshooting

### Common Issues

1. **"Coverage tools not found"**
   - Install lcov: `brew install lcov` (macOS) or equivalent

2. **Coverage data not generated**
   - Ensure `-DENABLE_COVERAGE=ON` is set
   - Clean and rebuild: `make clean && make`

3. **Low coverage on specific files**
   - Check if tests exist for those files
   - Look for platform-specific code
   - Verify test data covers all code paths

### Debugging Coverage

```bash
# Check if coverage files exist
ls -la *.gcno *.gcda

# Manual gcov analysis
gcov -b -c your_file.cpp

# Check lcov version
lcov --version
```

## Continuous Integration

For CI/CD pipelines, add coverage reporting:

```yaml
# Example GitHub Actions step
- name: Generate Coverage Report
  run: |
    cd build
    make coverage
    # Upload coverage report as artifact
```

## Resources

- [LCOV Documentation](http://ltp.sourceforge.net/coverage/lcov.php)
- [GCC Coverage Documentation](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)
- [Google Test Best Practices](https://github.com/google/googletest/blob/main/docs/primer.md)
