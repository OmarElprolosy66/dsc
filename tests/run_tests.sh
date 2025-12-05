#!/bin/bash
# Build and run DSC library tests
# Usage: ./run_tests.sh [compiler]
#   compiler: gcc (default), clang
#
# Automatically discovers and runs all test_*.c files

COMPILER="${1:-gcc}"

echo "============================================"
echo "Building DSC Library Tests"
echo "Compiler: $COMPILER"
echo "============================================"

cd "$(dirname "$0")"

mkdir -p build

# Find all test files
TEST_FILES=(test_*.c)

if [ ${#TEST_FILES[@]} -eq 0 ]; then
    echo "No test files found (test_*.c)"
    exit 1
fi

echo "Found ${#TEST_FILES[@]} test file(s)"
echo ""

# Compile all tests
BUILD_FAILED=0
for src in "${TEST_FILES[@]}"; do
    name="${src%.c}"
    echo "Compiling $src..."
    case "$COMPILER" in
        clang)
            clang -Wall -Wextra -o "build/$name" "$src" || BUILD_FAILED=1
            ;;
        *)
            gcc -Wall -Wextra -o "build/$name" "$src" || BUILD_FAILED=1
            ;;
    esac
done

if [ $BUILD_FAILED -ne 0 ]; then
    echo ""
    echo "Build FAILED!"
    exit 1
fi

echo ""
echo "============================================"
echo "Running Tests"
echo "============================================"

# Run all tests
TOTAL_FAILED=0
for src in "${TEST_FILES[@]}"; do
    name="${src%.c}"
    echo ""
    echo "Running $name..."
    echo ""
    ./build/"$name" || TOTAL_FAILED=$((TOTAL_FAILED + 1))
done

echo ""
echo "============================================"
if [ $TOTAL_FAILED -eq 0 ]; then
    echo "All ${#TEST_FILES[@]} test suite(s) passed!"
    exit 0
else
    echo "$TOTAL_FAILED of ${#TEST_FILES[@]} test suite(s) failed!"
    exit 1
fi
