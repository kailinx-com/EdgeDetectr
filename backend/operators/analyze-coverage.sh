#!/bin/bash

# Comprehensive coverage analysis script
set -e

echo "🔍 Analyzing test coverage comprehensively..."

cd build

# Create coverage directory
mkdir -p coverage

# Get test results summary
echo "📊 Test Results Summary:" > coverage/coverage_analysis.txt
echo "=========================" >> coverage/coverage_analysis.txt
echo "" >> coverage/coverage_analysis.txt

# Run tests and capture output
echo "🏃 Running tests to capture results..."
TEST_OUTPUT=$(./operators_test 2>&1)
echo "$TEST_OUTPUT" >> coverage/coverage_analysis.txt

# Extract test statistics
TOTAL_TESTS=$(echo "$TEST_OUTPUT" | grep "Running.*tests from.*test suites" | sed 's/.*Running \([0-9]*\) tests.*/\1/')
PASSED_TESTS=$(echo "$TEST_OUTPUT" | grep "PASSED.*tests" | sed 's/.*\[  PASSED  \] \([0-9]*\) tests.*/\1/')

echo "" >> coverage/coverage_analysis.txt
echo "## Test Statistics" >> coverage/coverage_analysis.txt
echo "Total tests: $TOTAL_TESTS" >> coverage/coverage_analysis.txt
echo "Passed tests: $PASSED_TESTS" >> coverage/coverage_analysis.txt
echo "Test success rate: $((PASSED_TESTS * 100 / TOTAL_TESTS))%" >> coverage/coverage_analysis.txt

# Analyze source files
echo "" >> coverage/coverage_analysis.txt
echo "## Source Code Analysis" >> coverage/coverage_analysis.txt
echo "========================" >> coverage/coverage_analysis.txt

SOURCE_FILES=(
    "../src/gradient/alt_sobel.cpp"
    "../src/gradient/ocv_sobel.cpp"
    "../src/gradient/omp_sobel.cpp"
    "../src/gradient/ocv_prewitt.cpp"
    "../src/gradient/ocv_roberts_cross.cpp"
    "../src/utils/image_utils.cpp"
)

TOTAL_LINES=0
TOTAL_FUNCTIONS=0
TOTAL_CLASSES=0

for file in "${SOURCE_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "" >> coverage/coverage_analysis.txt
        echo "### $file" >> coverage/coverage_analysis.txt
        
        # Count lines
        lines=$(wc -l < "$file")
        TOTAL_LINES=$((TOTAL_LINES + lines))
        echo "Lines of code: $lines" >> coverage/coverage_analysis.txt
        
        # Count functions (simple regex)
        functions=$(grep -c "^\s*[a-zA-Z_][a-zA-Z0-9_]*\s*[a-zA-Z_][a-zA-Z0-9_]*\s*(" "$file" 2>/dev/null || echo "0")
        TOTAL_FUNCTIONS=$((TOTAL_FUNCTIONS + functions))
        echo "Functions: $functions" >> coverage/coverage_analysis.txt
        
        # Count classes
        classes=$(grep -c "^\s*class\s" "$file" 2>/dev/null || echo "0")
        TOTAL_CLASSES=$((TOTAL_CLASSES + classes))
        echo "Classes: $classes" >> coverage/coverage_analysis.txt
        
        # Check if file has tests
        test_file=$(basename "$file" .cpp)
        test_file="test_${test_file}.cpp"
        if [ -f "../test/gradient/$test_file" ]; then
            echo "✅ Has dedicated test file: $test_file" >> coverage/coverage_analysis.txt
        else
            echo "❌ No dedicated test file found" >> coverage/coverage_analysis.txt
        fi
    fi
done

# Analyze test files
echo "" >> coverage/coverage_analysis.txt
echo "## Test Coverage Analysis" >> coverage/coverage_analysis.txt
echo "=========================" >> coverage/coverage_analysis.txt

TEST_FILES=(
    "../test/gradient/test_alt_sobel.cpp"
    "../test/gradient/test_ocv_sobel.cpp"
    "../test/gradient/test_omp_sobel.cpp"
    "../test/gradient/test_ocv_prewitt.cpp"
    "../test/gradient/test_ocv_roberts_cross.cpp"
    "../test/gradient/test_utils.cpp"
)

TOTAL_TEST_LINES=0
TOTAL_TEST_CASES=0

for test_file in "${TEST_FILES[@]}"; do
    if [ -f "$test_file" ]; then
        echo "" >> coverage/coverage_analysis.txt
        echo "### $test_file" >> coverage/coverage_analysis.txt
        
        # Count test lines
        test_lines=$(wc -l < "$test_file")
        TOTAL_TEST_LINES=$((TOTAL_TEST_LINES + test_lines))
        echo "Lines of test code: $test_lines" >> coverage/coverage_analysis.txt
        
        # Count test cases
        test_cases=$(grep -c "TEST_F\|TEST(" "$test_file" 2>/dev/null || echo "0")
        TOTAL_TEST_CASES=$((TOTAL_TEST_CASES + test_cases))
        echo "Test cases: $test_cases" >> coverage/coverage_analysis.txt
    fi
done

# Calculate coverage metrics
echo "" >> coverage/coverage_analysis.txt
echo "## Coverage Metrics" >> coverage/coverage_analysis.txt
echo "===================" >> coverage/coverage_analysis.txt
echo "Total source lines: $TOTAL_LINES" >> coverage/coverage_analysis.txt
echo "Total test lines: $TOTAL_TEST_LINES" >> coverage/coverage_analysis.txt
echo "Test-to-source ratio: $(echo "scale=2; $TOTAL_TEST_LINES / $TOTAL_LINES" | bc -l 2>/dev/null || echo "N/A")" >> coverage/coverage_analysis.txt
echo "Total functions: $TOTAL_FUNCTIONS" >> coverage/coverage_analysis.txt
echo "Total classes: $TOTAL_CLASSES" >> coverage/coverage_analysis.txt
echo "Total test cases: $TOTAL_TEST_CASES" >> coverage/coverage_analysis.txt

# Coverage assessment
echo "" >> coverage/coverage_analysis.txt
echo "## Coverage Assessment" >> coverage/coverage_analysis.txt
echo "======================" >> coverage/coverage_analysis.txt

if [ $TOTAL_TEST_CASES -gt 0 ] && [ $TOTAL_FUNCTIONS -gt 0 ]; then
    test_per_function=$(echo "scale=2; $TOTAL_TEST_CASES / $TOTAL_FUNCTIONS" | bc -l)
    echo "Test cases per function: $test_per_function" >> coverage/coverage_analysis.txt
    
    if (( $(echo "$test_per_function >= 2" | bc -l) )); then
        echo "✅ Good test density" >> coverage/coverage_analysis.txt
    elif (( $(echo "$test_per_function >= 1" | bc -l) )); then
        echo "⚠️  Moderate test density" >> coverage/coverage_analysis.txt
    else
        echo "❌ Low test density - consider adding more tests" >> coverage/coverage_analysis.txt
    fi
fi

if [ $TOTAL_TEST_LINES -gt 0 ] && [ $TOTAL_LINES -gt 0 ]; then
    test_ratio=$(echo "scale=2; $TOTAL_TEST_LINES * 100 / $TOTAL_LINES" | bc -l)
    echo "Test code percentage: ${test_ratio}%" >> coverage/coverage_analysis.txt
    
    if (( $(echo "$test_ratio >= 50" | bc -l) )); then
        echo "✅ Excellent test coverage ratio" >> coverage/coverage_analysis.txt
    elif (( $(echo "$test_ratio >= 30" | bc -l) )); then
        echo "✅ Good test coverage ratio" >> coverage/coverage_analysis.txt
    elif (( $(echo "$test_ratio >= 20" | bc -l) )); then
        echo "⚠️  Moderate test coverage ratio" >> coverage/coverage_analysis.txt
    else
        echo "❌ Low test coverage ratio - consider adding more tests" >> coverage/coverage_analysis.txt
    fi
fi

# Recommendations
echo "" >> coverage/coverage_analysis.txt
echo "## Recommendations" >> coverage/coverage_analysis.txt
echo "==================" >> coverage/coverage_analysis.txt

if [ $TOTAL_TEST_CASES -lt $TOTAL_FUNCTIONS ]; then
    echo "🔍 Consider adding tests for untested functions" >> coverage/coverage_analysis.txt
fi

if [ $TOTAL_TEST_LINES -lt $((TOTAL_LINES / 3)) ]; then
    echo "📝 Consider expanding test coverage with more comprehensive test cases" >> coverage/coverage_analysis.txt
fi

echo "🧪 All tests are passing, which indicates good code quality" >> coverage/coverage_analysis.txt
echo "📊 For detailed line-by-line coverage, consider using a different compiler or coverage tool" >> coverage/coverage_analysis.txt

# Generate HTML report
cat > coverage/index.html << EOF
<!DOCTYPE html>
<html>
<head>
    <title>Test Coverage Analysis Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; line-height: 1.6; }
        .header { background: #f0f0f0; padding: 20px; border-radius: 5px; margin-bottom: 20px; }
        .metric { background: #e8f5e8; padding: 15px; border-radius: 5px; margin: 10px 0; }
        .warning { background: #fff3cd; padding: 15px; border-radius: 5px; margin: 10px 0; }
        .error { background: #f8d7da; padding: 15px; border-radius: 5px; margin: 10px 0; }
        .section { margin: 20px 0; }
        .file { margin: 10px 0; padding: 10px; border-left: 4px solid #ddd; }
        pre { background: #f8f9fa; padding: 15px; border-radius: 5px; overflow-x: auto; }
    </style>
</head>
<body>
    <div class="header">
        <h1>🧪 Test Coverage Analysis Report</h1>
        <p>Generated on: $(date)</p>
        <p>This report provides a comprehensive analysis of your test coverage based on test results and source code analysis.</p>
    </div>
    
    <div class="metric">
        <h2>📊 Summary</h2>
        <p><strong>Total Tests:</strong> $TOTAL_TESTS</p>
        <p><strong>Passed Tests:</strong> $PASSED_TESTS</p>
        <p><strong>Success Rate:</strong> $((PASSED_TESTS * 100 / TOTAL_TESTS))%</p>
        <p><strong>Source Lines:</strong> $TOTAL_LINES</p>
        <p><strong>Test Lines:</strong> $TOTAL_TEST_LINES</p>
        <p><strong>Functions:</strong> $TOTAL_FUNCTIONS</p>
        <p><strong>Test Cases:</strong> $TOTAL_TEST_CASES</p>
    </div>
    
    <div class="section">
        <h2>📋 Detailed Analysis</h2>
        <p>See <a href="coverage_analysis.txt">coverage_analysis.txt</a> for the complete detailed analysis.</p>
    </div>
    
    <div class="section">
        <h2>🔧 Notes</h2>
        <p>This analysis is based on test execution results and source code examination.</p>
        <p>For line-by-line coverage analysis, consider using GCC instead of Clang, or a different coverage tool.</p>
    </div>
</body>
</html>
EOF

echo "✅ Comprehensive coverage analysis complete!"
echo "📁 HTML report: coverage/index.html"
echo "📄 Detailed analysis: coverage/coverage_analysis.txt"
echo ""
echo "📊 Summary:"
echo "   Total tests: $TOTAL_TESTS"
echo "   Passed tests: $PASSED_TESTS"
echo "   Source lines: $TOTAL_LINES"
echo "   Test lines: $TOTAL_TEST_LINES"
echo "   Functions: $TOTAL_FUNCTIONS"
echo "   Test cases: $TOTAL_TEST_CASES"
