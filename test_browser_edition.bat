@echo off
REM 🧪 Browser Edition Test Script

echo 🌐 NIGHTMARE BROWSER EDITION - TEST SUITE
echo ==========================================

echo 🔧 Compiling test suite...
g++ -std=c++17 -I. -DTEST_MODE=1 test_browser_edition.cpp -o test_browser_edition.exe

if %errorlevel% neq 0 (
    echo ❌ Compilation failed
    exit /b 1
)

echo ✅ Compilation successful

echo 🧪 Running tests...
test_browser_edition.exe

if %errorlevel% neq 0 (
    echo ❌ Tests failed
    exit /b 1
)

echo 🎯 Cleaning up...
del test_browser_edition.exe

echo.
echo 🏆 Browser Edition test suite completed successfully!
echo 🌐 Ready for WebAssembly build!