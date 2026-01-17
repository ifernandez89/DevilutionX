@echo off
echo 🌐 Starting local HTTP server for WebAssembly testing...
echo ================================================================
echo.
echo 🎯 FASE 1 Testing:
echo    - Open: http://localhost:8000
echo    - Expected: WebAssembly runtime loads (black screen is OK!)
echo    - Goal: No crashes, main() executes
echo.
echo Press Ctrl+C to stop server
echo ================================================================

REM Try Python 3 first, then Python 2, then Node.js
python -m http.server 8000 2>nul || python -m SimpleHTTPServer 8000 2>nul || npx http-server -p 8000 || echo ❌ No HTTP server available (install Python or Node.js)

pause