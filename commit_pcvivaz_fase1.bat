@echo off
cd /d "d:\Projects\DevilutionX"

echo ========================================
echo Agregando archivos al staging area...
echo ========================================

git add CHANGELOG.md
git add "Packaging/emscripten/nes/app.js"
git add "Packaging/emscripten/nes/index.html"
git add PC_VIVAZ_FASE1.1_DEBUG_SYSTEM.md
git add PC_VIVAZ_FASE1_SUMMARY.md
git add "documentacion imporante/PC_VIVAZ_ARQUITECTURA_INPUT.md"
git add "documentacion imporante/PC_VIVAZ_DEBUG_FLOWCHART.md"
git add "documentacion imporante/PC_VIVAZ_FASE1_FIX_INPUT.md"
git add "documentacion imporante/PC_VIVAZ_QUICK_REFERENCE.md"
git add "documentacion imporante/PC_VIVAZ_README.md"
git add "documentacion imporante/PC_VIVAZ_RESUMEN_EJECUTIVO.md"

echo.
echo ========================================
echo Estado de Git:
echo ========================================
git status

echo.
echo ========================================
echo Creando commit...
echo ========================================

git commit -m "feat(pc-vivaz): Add FASE 1 input fix and visual debug system

- FASE 1: Fix Subor Keyboard input (Device P5 = 1539)
- FASE 1: Conditional preventDefault() for keyboard events
- FASE 1.1: Add visual debug panel for real-time diagnostics
- FASE 1.1: Add comprehensive console logging system
- Docs: 6 technical documents for PC Vivaz implementation
- Cache-busting for app.js to force reload

This enables PC Vivaz / Educational Computer 2000 keyboard input
and provides full debugging visibility for validation."

echo.
echo ========================================
echo Pusheando a origin master...
echo ========================================

git push origin master

echo.
echo ========================================
echo COMPLETADO!
echo ========================================
echo.
echo Para probar en GitHub Pages:
echo https://[tu-usuario].github.io/[tu-repo]/Packaging/emscripten/nes/
echo.

pause
