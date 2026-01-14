@echo off
echo ========================================
echo APOCALYPSE RAPID-FIRE FIX TEST
echo Enero 13, 2026
echo ========================================
echo.
echo TESTING INSTRUCTIONS:
echo 1. Game will launch with architectural logging enabled
echo 2. Create a high-level sorcerer character
echo 3. Learn Apocalypse spell
echo 4. Go to a dungeon level with many monsters
echo 5. RAPIDLY CLICK Apocalypse 10+ times (fast-click test)
echo 6. Watch for crashes (should be ZERO)
echo 7. Check logs for protection messages
echo.
echo EXPECTED BEHAVIOR:
echo - Maximum 1 Apocalypse every 3 frames
echo - Maximum 2 Apocalypse per second
echo - NO CRASHES even with extreme fast-clicking
echo - Protection logs showing "atomic check" blocks
echo - Protection logs showing "delayed unlock" after 3 frames
echo.
echo CRITICAL SUCCESS CRITERIA:
echo - 0%% crash rate
echo - Responsive gameplay (500ms feels natural)
echo - Protection logs show delayed unlock working
echo - ClearApocalypseInProgress calls are IGNORED
echo.
echo Press any key to launch game...
pause > nul

echo.
echo Launching DevilutionX Nightmare Edition...
echo Logs will be in: build_NOW\debug_logs\architectural_analysis.log
echo.

devilutionx.exe

echo.
echo ========================================
echo TEST COMPLETE
echo ========================================
echo.
echo VERIFICATION STEPS:
echo 1. Did the game crash? (Should be NO)
echo 2. Check logs: build_NOW\debug_logs\architectural_analysis.log
echo 3. Look for "Apocalypse already in progress" messages
echo 4. Look for "delayed unlock" messages
echo 5. Count Apocalypse casts - should be max 2 per second
echo.
echo If NO CRASHES and logs show protection working:
echo ✅ FIX SUCCESSFUL - Delayed unlock system working
echo.
echo If CRASHES or multiple casts per frame:
echo ❌ FIX FAILED - Review logs and code
echo.
pause
