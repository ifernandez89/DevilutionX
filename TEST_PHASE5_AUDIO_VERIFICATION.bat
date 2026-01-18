@echo off
echo 🔊 FASE 5: Audio System Verification Testing
echo ================================================================
echo.
echo 📋 Testing Plan:
echo    1. Start local web server for WebAssembly
echo    2. Launch browser with audio verification
echo    3. Monitor audio system initialization
echo    4. Test sound effects and audio playback
echo    5. Verify logging output
echo.

REM Change to phase5 directory
cd phase5

REM Start simple HTTP server for WebAssembly
echo 🌐 Starting local web server...
echo    URL: http://localhost:8000
echo    Files: devilutionx.js, devilutionx.wasm, devilutionx.data
echo.

REM Check if Python is available for simple server
python --version >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo 🐍 Using Python HTTP server...
    start "DevilutionX Phase5 Server" python -m http.server 8000
) else (
    REM Try Node.js if Python not available
    node --version >nul 2>&1
    if %ERRORLEVEL% equ 0 (
        echo 📦 Using Node.js HTTP server...
        echo const http = require('http'); > server.js
        echo const fs = require('fs'); >> server.js
        echo const path = require('path'); >> server.js
        echo const server = http.createServer((req, res) => { >> server.js
        echo   const filePath = path.join(__dirname, req.url === '/' ? 'index.html' : req.url); >> server.js
        echo   fs.readFile(filePath, (err, data) => { >> server.js
        echo     if (err) { res.writeHead(404); res.end('Not found'); return; } >> server.js
        echo     res.writeHead(200); res.end(data); >> server.js
        echo   }); >> server.js
        echo }); >> server.js
        echo server.listen(8000, () => console.log('Server running on http://localhost:8000')); >> server.js
        start "DevilutionX Phase5 Server" node server.js
    ) else (
        echo ❌ Neither Python nor Node.js found for HTTP server
        echo    Please install Python or Node.js to run WebAssembly version
        pause
        exit /b 1
    )
)

REM Wait a moment for server to start
timeout /t 3 /nobreak >nul

REM Open browser
echo 🌐 Opening browser for audio verification testing...
start http://localhost:8000

echo.
echo 🎯 TESTING INSTRUCTIONS:
echo ================================================================
echo 1. Wait for the game to load in the browser
echo 2. Check browser console (F12) for Phase 5 audio logs
echo 3. Look for messages starting with [FASE5-AUDIO]
echo 4. Test the following audio features:
echo    - Menu navigation sounds
echo    - Character selection audio
echo    - In-game sound effects
echo    - Background music
echo 5. Verify audio initialization messages
echo.
echo 📊 Expected Log Messages:
echo    [FASE5-AUDIO] 🔊 AUDIO INIT: SDL Audio Subsystem
echo    [FASE5-AUDIO] 🎵 AUDIO DEVICE: SDL Audio Device
echo    [FASE5-AUDIO] 📂 AUDIO LOAD: [sound files]
echo    [FASE5-AUDIO] ▶️ AUDIO PLAY: [sound effects]
echo.
echo Press any key when testing is complete...
pause