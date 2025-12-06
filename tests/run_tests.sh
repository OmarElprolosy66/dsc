#!/bin/bash
# Build and run DSC library tests
# Usage: ./run_tests.sh [compiler] [--valgrind [test_name]]
#   compiler: gcc (default), clang
#   --valgrind: Run tests with valgrind memory checker
#   test_name: Optional specific test to run with valgrind (e.g., test_hash_table)
#
# Examples:
#   ./run_tests.sh                              - Run all tests normally
#   ./run_tests.sh gcc --valgrind               - Run all tests with valgrind
#   ./run_tests.sh gcc --valgrind test_list     - Run only test_list with valgrind
#
# Automatically discovers and runs all test_*.c files

COMPILER=""
VALGRIND=0
VALGRIND_TEST=""

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --valgrind)
            VALGRIND=1
            shift
            if [[ -n "$1" && "$1" != --* ]]; then
                VALGRIND_TEST="$1"
                shift
            fi
            ;;
        *)
            if [[ -z "$COMPILER" ]]; then
                COMPILER="$1"
            fi
            shift
            ;;
    esac
done

# Set default compiler
COMPILER="${COMPILER:-gcc}"

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
if [ $VALGRIND -eq 1 ]; then
    echo "Running Tests with Valgrind"
else
    echo "Running Tests"
fi
echo "============================================"

# Run all tests
TOTAL_FAILED=0

if [ $VALGRIND -eq 1 ]; then
    # Check if valgrind is available
    if ! command -v valgrind &> /dev/null; then
        echo "ERROR: valgrind is not installed. Please install it first."
        echo "  Ubuntu/Debian: sudo apt-get install valgrind"
        echo "  macOS: brew install valgrind"
        exit 1
    fi
    
    if [ -n "$VALGRIND_TEST" ]; then
        # Run specific test with valgrind
        echo ""
        echo "Running $VALGRIND_TEST with valgrind..."
        echo ""
        valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/"$VALGRIND_TEST" || TOTAL_FAILED=1
        TEST_COUNT=1
    else
        # Run all tests with valgrind
        TEST_COUNT=${#TEST_FILES[@]}
        for src in "${TEST_FILES[@]}"; do
            name="${src%.c}"
            echo ""
            echo "Running $name with valgrind..."
            echo ""
            valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./build/"$name" || TOTAL_FAILED=$((TOTAL_FAILED + 1))
        done
    fi
else
    # Run all tests normally
    TEST_COUNT=${#TEST_FILES[@]}
    for src in "${TEST_FILES[@]}"; do
        name="${src%.c}"
        echo ""
        echo "Running $name..."
        echo ""
        ./build/"$name" || TOTAL_FAILED=$((TOTAL_FAILED + 1))
    done
fi

echo ""
echo "============================================"
if [ $TOTAL_FAILED -eq 0 ]; then
    echo "All $TEST_COUNT test suite(s) passed!"
    exit 0
else
    echo "$TOTAL_FAILED of $TEST_COUNT test suite(s) failed!"
    exit 1
fi
