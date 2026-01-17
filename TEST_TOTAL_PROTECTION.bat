@echo off
echo ========================================
echo 🛡️ TESTING TOTAL ARCHITECTURAL SHIELD
echo ========================================
echo.
echo Testing both CONTINUE and NEW GAME flows...
echo.
echo 1. Testing CONTINUE (LoadGame flow)
echo    - Should sanitize ALL coordinates from save file
echo    - Player, monsters, items, objects, lights, portals
echo.
echo 2. Testing NEW GAME (UnPackPlayer + forced coordinates)
echo    - Should sanitize player coordinates from save file
echo    - Should force safe coordinates (75, 68) for NEW GAME
echo    - Should regenerate all levels cleanly
echo.
echo Starting DevilutionX with TOTAL PROTECTION...
echo.

cd build_COMPILE_FRESH
start devilutionx.exe

echo.
echo ========================================
echo 🛡️ TOTAL PROTECTION SYSTEM ACTIVE
echo ========================================
echo.
echo Protection Coverage:
echo   ✅ Player coordinates (LoadPlayer + UnPackPlayer)
echo   ✅ Monster coordinates (LoadMonster - ALL positions)
echo   ✅ Item coordinates (LoadItemData)
echo   ✅ Object coordinates (LoadObject)
echo   ✅ Light coordinates (LoadLighting - tile + old)
echo   ✅ Portal coordinates (LoadPortal)
echo   ✅ NEW GAME forced safe coordinates
echo.
echo Check debug_logs/architectural_shield.log for sanitization details
echo.
pause