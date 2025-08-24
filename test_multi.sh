#!/bin/bash

# Counter for failed tests
fail_count=0
total_runs=10000

echo "Running tests $total_runs times. Only failures will be displayed."
echo "Press Ctrl+C to stop at any time."
echo ""

for i in $(seq 1 $total_runs); do
    # Run the test and capture output
    output=$(bin/Debug-linux-x86_64/tests/tests 2>&1)
    exit_code=$?
    
    # Check if test failed
    if [ $exit_code -ne 0 ]; then
        fail_count=$((fail_count + 1))
        echo "FAILURE on run $i:"
        echo "$output"
        echo "----------------------------------------"
    fi
    
    # Show progress every 100 runs
    if [ $((i % 100)) -eq 0 ]; then
        echo "Completed $i runs, $fail_count failures so far"
    fi
done

echo ""
echo "Test completed:"
echo "Total runs: $total_runs"
echo "Failures: $fail_count"