#!/bin/bash

# Simple coverage report script that works around LLVM gcov issues
set -e

echo "🔍 Generating simple coverage report..."

cd build

# Check if coverage data exists
if [ ! -f "coverage.info" ]; then
    echo "⚠️  No coverage data found. Running tests first..."
    ./operators_test
fi

# Create coverage directory
mkdir -p coverage

# Generate simple coverage report
echo "📊 Analyzing coverage data..."

# Get list of source files
SOURCE_FILES=(
    "../src/gradient/alt_sobel.cpp"
    "../src/gradient/ocv_sobel.cpp"
    "../src/gradient/omp_sobel.cpp"
    "../src/gradient/ocv_prewitt.cpp"
    "../src/gradient/ocv_roberts_cross.cpp"
    "../src/utils/image_utils.cpp"
)

echo "# Test Coverage Report" > coverage/coverage_report.txt
echo "Generated on: $(date)" >> coverage/coverage_report.txt
echo "" >> coverage/coverage_report.txt

TOTAL_LINES=0
COVERED_LINES=0

for file in "${SOURCE_FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "📁 Analyzing: $file"
        
        # Count total lines in source file
        file_lines=$(wc -l < "$file")
        TOTAL_LINES=$((TOTAL_LINES + file_lines))
        
        # Try to get coverage info (this might fail with LLVM gcov)
        if gcov -b -c "$file" 2>/dev/null; then
            # Extract coverage percentage from gcov output
            coverage_percent=$(gcov -b -c "$file" 2>/dev/null | grep "Lines executed:" | sed 's/.*: \([0-9.]*\)% .*/\1/')
            if [ ! -z "$coverage_percent" ]; then
                covered_lines=$(echo "$file_lines * $coverage_percent / 100" | bc -l | cut -d. -f1)
                COVERED_LINES=$((COVERED_LINES + covered_lines))
                echo "✅ $file: $coverage_percent% coverage" >> coverage/coverage_report.txt
            else
                echo "❌ $file: Coverage data unavailable" >> coverage/coverage_report.txt
            fi
        else
            echo "❌ $file: Coverage analysis failed" >> coverage/coverage_report.txt
        fi
        
        # Clean up gcov files
        rm -f *.gcov 2>/dev/null || true
    fi
done

echo "" >> coverage/coverage_report.txt
echo "## Summary" >> coverage/coverage_report.txt
echo "Total source lines: $TOTAL_LINES" >> coverage/coverage_report.txt

if [ $TOTAL_LINES -gt 0 ]; then
    overall_coverage=$(echo "scale=1; $COVERED_LINES * 100 / $TOTAL_LINES" | bc -l)
    echo "Overall coverage: ${overall_coverage}%" >> coverage/coverage_report.txt
    
    # Create a simple HTML report
    cat > coverage/index.html << EOF
<!DOCTYPE html>
<html>
<head>
    <title>Test Coverage Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; }
        .header { background: #f0f0f0; padding: 20px; border-radius: 5px; }
        .coverage { font-size: 24px; color: #2c5aa0; }
        .file { margin: 10px 0; padding: 10px; border-left: 4px solid #ddd; }
        .good { border-left-color: #4caf50; }
        .poor { border-left-color: #f44336; }
        .summary { background: #e8f5e8; padding: 20px; border-radius: 5px; margin: 20px 0; }
    </style>
</head>
<body>
    <div class="header">
        <h1>🧪 Test Coverage Report</h1>
        <p>Generated on: $(date)</p>
    </div>
    
    <div class="summary">
        <h2>📊 Overall Coverage: ${overall_coverage}%</h2>
        <p>Total source lines: $TOTAL_LINES</p>
        <p>Covered lines: $COVERED_LINES</p>
    </div>
    
    <h2>📁 File Coverage Details</h2>
EOF

    # Add file details to HTML
    for file in "${SOURCE_FILES[@]}"; do
        if [ -f "$file" ]; then
            echo "    <div class='file'>" >> coverage/index.html
            echo "        <strong>$file</strong><br>" >> coverage/index.html
            echo "        <span>Coverage: Available in text report</span>" >> coverage/index.html
            echo "    </div>" >> coverage/index.html
        fi
    done

    cat >> coverage/index.html << EOF
    
    <h2>📋 Detailed Report</h2>
    <p>See <a href="coverage_report.txt">coverage_report.txt</a> for detailed analysis.</p>
    
    <h2>🔧 Notes</h2>
    <p>This is a simplified coverage report due to LLVM gcov compatibility issues.</p>
    <p>For more detailed analysis, consider using a different compiler or coverage tool.</p>
</body>
</html>
EOF

    echo "✅ Coverage report generated!"
    echo "📁 HTML report: coverage/index.html"
    echo "📄 Text report: coverage/coverage_report.txt"
    echo "📊 Overall coverage: ${overall_coverage}%"
else
    echo "❌ No source files found for analysis"
fi
