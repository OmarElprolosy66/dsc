@echo off
REM Build and run DSC library tests
REM Usage: run_tests.bat [compiler] [--valgrind [test_name]]
REM   compiler: gcc (default), clang, or cl (MSVC)
REM   --valgrind: Run valgrind (requires WSL on Windows)
REM   test_name: Optional specific test to run with valgrind (e.g., test_hash_table)
REM
REM Examples:
REM   run_tests.bat                              - Run all tests normally
REM   run_tests.bat gcc --valgrind               - Run all tests with valgrind
REM   run_tests.bat gcc --valgrind test_list     - Run only test_list with valgrind
REM
REM Automatically discovers and runs all test_*.c files

setlocal enabledelayedexpansion

set "COMPILER="
set "VALGRIND="
set "VALGRIND_TEST="

REM Parse arguments
:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="--valgrind" (
    set "VALGRIND=1"
    if not "%~2"=="" (
        if not "%~2:~0,2%"=="--" (
            set "VALGRIND_TEST=%~2"
            shift
        )
    )
    shift
    goto parse_args
)
if "!COMPILER!"=="" (
    set "COMPILER=%~1"
    shift
    goto parse_args
)
shift
goto parse_args

:args_done
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
if "!VALGRIND!"=="1" (
    echo Running Tests with Valgrind ^(WSL^)
) else (
    echo Running Tests
)
echo ============================================

REM Run all tests
set TOTAL_FAILED=0

if "!VALGRIND!"=="1" (
    REM Check if WSL is available
    wsl --version >nul 2>&1
    if errorlevel 1 (
        echo ERROR: WSL is not available. Valgrind requires WSL on Windows.
        exit /b 1
    )
    
    REM Get the current directory in WSL format
    for /f "delims=" %%i in ('wsl wslpath -a "%CD%"') do set "WSL_PATH=%%i"
    
    if "!VALGRIND_TEST!"=="" (
        REM Run all tests with valgrind
        for %%f in (test_*.c) do (
            echo.
            echo Running %%~nf with valgrind...
            echo.
            wsl bash -c "cd '!WSL_PATH!/build' && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./%%~nf"
            if errorlevel 1 set /a TOTAL_FAILED+=1
        )
    ) else (
        REM Run specific test with valgrind
        echo.
        echo Running !VALGRIND_TEST! with valgrind...
        echo.
        wsl bash -c "cd '!WSL_PATH!/build' && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./!VALGRIND_TEST!"
        if errorlevel 1 set TOTAL_FAILED=1
        set TEST_COUNT=1
    )
) else {
    REM Run all tests normally
    for %%f in (test_*.c) do (
        echo.
        echo Running %%~nf...
        echo.
        build\%%~nf.exe
        if errorlevel 1 set /a TOTAL_FAILED+=1
    )
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
