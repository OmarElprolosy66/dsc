@echo off
REM Build and run DSC library tests
REM Usage: run_tests.bat [compiler]
REM   compiler: gcc (default), clang, or cl (MSVC)
REM
REM Automatically discovers and runs all test_*.c files

setlocal enabledelayedexpansion

set "COMPILER=%~1"
if "!COMPILER!"=="" set "COMPILER=gcc"

echo ============================================
echo Building DSC Library Tests
echo Compiler: !COMPILER!
echo ============================================

cd /d "%~dp0"

if not exist "build" mkdir build

REM Check if any test files exist
dir /b test_*.c >nul 2>&1
if errorlevel 1 (
    echo No test files found ^(test_*.c^)
    exit /b 1
)

REM Count test files
set TEST_COUNT=0
for %%f in (test_*.c) do set /a TEST_COUNT+=1
echo Found !TEST_COUNT! test file^(s^)
echo.

REM Compile all tests
set BUILD_FAILED=0
for %%f in (test_*.c) do (
    echo Compiling %%f...
    if "!COMPILER!"=="cl" (
        cl /nologo /W4 /Fe:build\%%~nf.exe %%f
    ) else if "!COMPILER!"=="clang" (
        clang -Wall -Wextra -o build\%%~nf.exe %%f
    ) else (
        gcc -Wall -Wextra -o build\%%~nf.exe %%f
    )
    if errorlevel 1 set BUILD_FAILED=1
)

if !BUILD_FAILED! neq 0 (
    echo.
    echo Build FAILED!
    exit /b 1
)

echo.
echo ============================================
echo Running Tests
echo ============================================

REM Run all tests
set TOTAL_FAILED=0
for %%f in (test_*.c) do (
    echo.
    echo Running %%~nf...
    echo.
    build\%%~nf.exe
    if errorlevel 1 set /a TOTAL_FAILED+=1
)

echo.
echo ============================================
if !TOTAL_FAILED! equ 0 (
    echo All !TEST_COUNT! test suite^(s^) passed!
    exit /b 0
) else (
    echo !TOTAL_FAILED! of !TEST_COUNT! test suite^(s^) failed!
    exit /b 1
)
