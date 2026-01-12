@echo off
echo ========================================
echo COMPILING APOCALYPSE FIX - SIMPLIFIED
echo ========================================
echo.
echo Changes made:
echo 1. ProcessApocalypse - Reverted to simple original logic
echo 2. AddApocalypse - Removed all artificial limits  
echo 3. CanSafelyCastApocalypse - Always returns true
echo 4. CastSpell - Completely original without any protections
echo 5. DoSpell - Completely original without any protections
echo.
echo Root cause: Over-engineering with competing protection systems
echo Solution: Simplified back to DevilutionX original logic
echo.

REM Try to use existing build
if exist "build_NOW\Makefile" (
    echo Using existing build_NOW...
    cd build_NOW
    mingw32-make -j 4
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ========================================
        echo COMPILATION SUCCESSFUL!
        echo ========================================
        echo Executable: build_NOW\devilutionx.exe
        echo.
        echo TEST APOCALYPSE:
        echo 1. Run the game
        echo 2. Create a Sorcerer
        echo 3. Get Apocalypse spell
        echo 4. Cast it multiple times
        echo 5. Should work without crashes!
        echo.
        pause
        exit /b 0
    )
    cd ..
)

REM Try build_final
if exist "build_final\Makefile" (
    echo Using build_final...
    cd build_final
    mingw32-make -j 4
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ========================================
        echo COMPILATION SUCCESSFUL!
        echo ========================================
        echo Executable: build_final\devilutionx.exe
        echo.
        echo TEST APOCALYPSE:
        echo 1. Run the game
        echo 2. Create a Sorcerer  
        echo 3. Get Apocalypse spell
        echo 4. Cast it multiple times
        echo 5. Should work without crashes!
        echo.
        pause
        exit /b 0
    )
    cd ..
)

echo.
echo ========================================
echo COMPILATION FAILED
echo ========================================
echo Need to fix build system issues first
pause