# Testing Guide

## Running Tests

### Basic Usage

Run all tests with default compiler (gcc):

```bash
# Linux/macOS
./run_tests.sh

# Windows
run_tests.bat
```

### Custom Compiler

```bash
# Linux/macOS
./run_tests.sh clang

# Windows
run_tests.bat cl      # MSVC
run_tests.bat clang
```

## Memory Leak Detection (Valgrind)

### Run All Tests with Valgrind

```bash
# Linux/macOS
./run_tests.sh --valgrind
./run_tests.sh gcc --valgrind

# Windows (requires WSL)
run_tests.bat --valgrind
run_tests.bat gcc --valgrind
```

**Sample Output:**
```
============================================
Running Tests with Valgrind
============================================

Running test_hash_table with valgrind...

=== Hash Table Tests ===
[PASS] hash_table_init_basic
...
========================================
Tests run: 29, Passed: 29, Failed: 0
All tests passed!
========================================

HEAP SUMMARY:
    in use at exit: 0 bytes in 0 blocks
  total heap usage: 2,087 allocs, 2,087 frees, 163,779 bytes allocated

All heap blocks were freed -- no leaks are possible

ERROR SUMMARY: 0 errors from 0 contexts
```

### Run Specific Test with Valgrind

```bash
# Linux/macOS
./run_tests.sh --valgrind test_list
./run_tests.sh gcc --valgrind test_hash_table

# Windows (requires WSL)
run_tests.bat --valgrind test_list
run_tests.bat gcc --valgrind test_utilities
```

## Test Results

**All 139 tests pass with zero memory leaks:**

| Test Suite | Tests | Allocations | Frees | Result |
|------------|-------|-------------|-------|--------|
| test_hash_table | 29 | 2,087 | 2,087 | No leaks |
| test_list | 40 | 48 | 48 | No leaks |
| test_set | 42 | 4,264 | 4,264 | No leaks |
| test_utilities | 28 | 143 | 143 | No leaks |
| **Total** | **139** | **6,542** | **6,542** | **0 errors** |
## Test Framework

Tests use a custom lightweight framework defined in `test_framework.h`:

```c
#include "test_framework.h"

#define DSC_IMPLEMENTATION
#include "../dsc.h"

TEST(my_test_name) {
    // Test code
    ASSERT_EQ(expected, actual);
    ASSERT_TRUE(condition);
    ASSERT_NOT_NULL(ptr);
}

int main(void) {
    TEST_INIT();  /* Enable ANSI colors on Windows */
    TEST_HEADER("My Tests");
    
    TEST_SECTION("Feature Tests");
    RUN_TEST(my_test_name);
    
    TEST_SUMMARY();
    return TEST_EXIT_CODE();
}
```

### Available Assertions

- `ASSERT(condition)` - Assert condition is true
- `ASSERT_TRUE(condition)` - Same as ASSERT
- `ASSERT_FALSE(condition)` - Assert condition is false
- `ASSERT_EQ(expected, actual)` - Assert equality
- `ASSERT_STR_EQ(expected, actual)` - Assert string equality
- `ASSERT_NULL(ptr)` - Assert pointer is NULL
- `ASSERT_NOT_NULL(ptr)` - Assert pointer is not NULL

### Test Macros

- `TEST(name)` - Define a test
- `RUN_TEST(name)` - Run a test
- `TEST_INIT()` - Initialize framework (enables colors on Windows)
- `TEST_HEADER("name")` - Print main test header
- `TEST_SECTION("name")` - Print section header
- `TEST_SUMMARY()` - Print test results summary
- `TEST_EXIT_CODE()` - Return appropriate exit code

## Adding New Tests

1. Create `test_feature.c` in the `tests/` directory
2. Follow the test framework pattern (see above)
3. Tests are automatically discovered and compiled
4. Run `./run_tests.sh` to verify

Example:

```c
/**
 * Feature Tests
 * Tests all functionality of the new feature.
 */

#include "test_framework.h"

#define DSC_IMPLEMENTATION
#include "../dsc.h"

TEST(feature_basic) {
    // Your test code
    ASSERT_TRUE(1 == 1);
}

int main(void) {
    TEST_INIT();
    TEST_HEADER("Feature Tests");
    
    TEST_SECTION("Basic Tests");
    RUN_TEST(feature_basic);
    
    TEST_SUMMARY();
    return TEST_EXIT_CODE();
}
```

## Valgrind Options Explained

The test scripts use these valgrind options:

- `--leak-check=full` - Detailed memory leak information
- `--show-leak-kinds=all` - Show all types of leaks (definite, indirect, possible, reachable)
- `--track-origins=yes` - Track origins of uninitialized values

For more valgrind options, see: `man valgrind` or https://valgrind.org/docs/manual/manual.html

## Windows WSL Setup

To use valgrind on Windows:

1. Install WSL: `wsl --install`
2. Install valgrind in WSL:
   ```bash
   sudo apt-get update
   sudo apt-get install valgrind
   ```
3. Run tests: `run_tests.bat --valgrind`

## Continuous Integration

The test suite is designed to work with CI/CD pipelines:

```yaml
# Example GitHub Actions
- name: Run tests
  run: cd tests && ./run_tests.sh

- name: Run tests with valgrind
  run: cd tests && ./run_tests.sh --valgrind
```

Exit codes:
- `0` - All tests passed
- `1` - One or more tests failed
