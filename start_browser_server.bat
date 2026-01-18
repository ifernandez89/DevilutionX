@echo off
echo 🌐 Starting Diablo 1 Browser Edition Server
echo.

REM Check if Python is available
python --version >nul 2>&1
if %errorlevel% == 0 (
    echo ✅ Python found - Starting HTTP server on port 8000
    echo.
    echo 🎮 Open your browser and go to:
    echo    http://localhost:8000/nightmare_browser_WORKING_FINAL.html
    echo.
    echo Press Ctrl+C to stop the server
    echo.
    python -m http.server 8000
) else (
    echo ❌ Python not found. Trying alternative method...
    echo.
    
    REM Try Node.js if available
    node --version >nul 2>&1
    if %errorlevel% == 0 (
        echo ✅ Node.js found - Installing and starting server
        npm install -g http-server
        echo.
        echo 🎮 Open your browser and go to:
        echo    http://localhost:8080/nightmare_browser_WORKING_FINAL.html
        echo.
        http-server -p 8080
    ) else (
        echo ❌ Neither Python nor Node.js found
        echo.
        echo 📋 Manual Setup Instructions:
        echo 1. Install Python from https://python.org
        echo 2. Run this script again
        echo.
        echo OR use any other web server like:
        echo - XAMPP, WAMP, or MAMP
        echo - Live Server extension in VS Code
        echo - Any other local web server
        echo.
        pause
    )
)