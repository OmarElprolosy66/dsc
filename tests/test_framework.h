/**
 * Minimal C Testing Framework for DSC Library
 * 
 * Copyright (C) 2025 OmarElprolosy66
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Usage:
 *   TEST(test_name) {
 *       ASSERT(condition);
 *       ASSERT_EQ(expected, actual);
 *       ASSERT_STR_EQ(expected, actual);
 *       ASSERT_NULL(ptr);
 *       ASSERT_NOT_NULL(ptr);
 *   }
 *   
 *   int main(void) {
 *       RUN_TEST(test_name);
 *       TEST_SUMMARY();
 *       return TEST_EXIT_CODE();
 *   }
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Enable ANSI colors on Windows 10+ */
#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    
    static inline void _test_enable_ansi_colors(void) {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE) {
            DWORD mode = 0;
            if (GetConsoleMode(hOut, &mode)) {
                /* ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004 */
                SetConsoleMode(hOut, mode | 0x0004);
            }
        }
    }
    #define TEST_INIT() _test_enable_ansi_colors()
#else
    #define TEST_INIT() ((void)0)
#endif

/* ANSI color codes - work on all modern terminals */
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_RESET   "\033[0m"

/* Test counters */
static int _tests_run                 = 0;
static int _tests_passed              = 0;
static int _tests_failed              = 0;
static int _current_test_failed       = 0;
static const char* _current_test_name = NULL;

/* Test definition macro */
#define TEST(name) \
    static void name##_impl(void); \
    static void name(void) { \
        _current_test_name = #name; \
        _current_test_failed = 0; \
        _tests_run++; \
        name##_impl(); \
        if (!_current_test_failed) { \
            _tests_passed++; \
            printf(COLOR_GREEN "[PASS]" COLOR_RESET " %s\n", #name); \
        } \
    } \
    static void name##_impl(void)

/* Run a test */
#define RUN_TEST(name) name()

/* Assertion macros */
#define ASSERT(condition) do { \
    if (!(condition)) { \
        printf(COLOR_RED "[FAIL]" COLOR_RESET " %s\n", _current_test_name); \
        printf("       Assertion failed: %s\n", #condition); \
        printf("       at %s:%d\n", __FILE__, __LINE__); \
        _current_test_failed = 1; \
        _tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(expected, actual) do { \
    long long _exp = (long long)(expected); \
    long long _act = (long long)(actual); \
    if (_exp != _act) { \
        printf(COLOR_RED "[FAIL]" COLOR_RESET " %s\n", _current_test_name); \
        printf("       Expected: %lld, Actual: %lld\n", _exp, _act); \
        printf("       at %s:%d\n", __FILE__, __LINE__); \
        _current_test_failed = 1; \
        _tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_STR_EQ(expected, actual) do { \
    const char* _exp = (expected); \
    const char* _act = (actual); \
    if (_exp == NULL || _act == NULL || strcmp(_exp, _act) != 0) { \
        printf(COLOR_RED "[FAIL]" COLOR_RESET " %s\n", _current_test_name); \
        printf("       Expected: \"%s\", Actual: \"%s\"\n", _exp ? _exp : "(null)", _act ? _act : "(null)"); \
        printf("       at %s:%d\n", __FILE__, __LINE__); \
        _current_test_failed = 1; \
        _tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_NULL(ptr) do { \
    if ((ptr) != NULL) { \
        printf(COLOR_RED "[FAIL]" COLOR_RESET " %s\n", _current_test_name); \
        printf("       Expected NULL, got non-NULL pointer\n"); \
        printf("       at %s:%d\n", __FILE__, __LINE__); \
        _current_test_failed = 1; \
        _tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_NOT_NULL(ptr) do { \
    if ((ptr) == NULL) { \
        printf(COLOR_RED "[FAIL]" COLOR_RESET " %s\n", _current_test_name); \
        printf("       Expected non-NULL, got NULL\n"); \
        printf("       at %s:%d\n", __FILE__, __LINE__); \
        _current_test_failed = 1; \
        _tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_TRUE(condition)  ASSERT(condition)
#define ASSERT_FALSE(condition) ASSERT(!(condition))

/* Test summary */
#define TEST_SUMMARY() do { \
    printf("\n========================================\n"); \
    printf("Tests run: %d, Passed: %d, Failed: %d\n", \
           _tests_run, _tests_passed, _tests_failed); \
    if (_tests_failed == 0) { \
        printf(COLOR_GREEN "All tests passed!" COLOR_RESET "\n"); \
    } else { \
        printf(COLOR_RED "%d test(s) failed." COLOR_RESET "\n", _tests_failed); \
    } \
    printf("========================================\n"); \
} while(0)

/* Exit code based on test results */
#define TEST_EXIT_CODE() (_tests_failed > 0 ? 1 : 0)

/* Section header for organizing tests */
#define TEST_SECTION(name) \
    printf("\n--- %s ---\n", name)

#endif /* TEST_FRAMEWORK_H */
