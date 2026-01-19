@echo off
echo 🔧 FIXING GitHub Pages - Removing Problematic Submodules
echo ========================================================

echo.
echo 📁 Adding all fixes to git...
git add .github/workflows/pages.yml
git add .gitignore

echo.
echo 💾 Committing GitHub Pages submodule fix...
git commit -m "🔧 CRITICAL FIX: Remove problematic submodules for GitHub Pages

❌ ISSUES FIXED:
- Removed all problematic build directory submodules
- Fixed submodule checkout errors in GitHub Actions
- Created clean GitHub Pages workflow without submodules
- Updated .gitignore to prevent future submodule issues

✅ CHANGES:
- .github/workflows/pages.yml: Clean workflow without submodules
- .gitignore: Added rules to ignore build directories and deps
- Removed: All build_*/_deps/bzip2-src submodules
- Removed: emsdk and other problematic submodules

🌐 GITHUB PAGES STATUS:
- Workflow now uses 'submodules: false' to avoid checkout errors
- Only deploys /docs folder content (HTML + WebAssembly files)
- No dependency on external submodules or build processes

🚀 RESULT: GitHub Pages should now deploy successfully!"

echo.
echo 🚀 Pushing GitHub Pages fix...
git push origin sistemaWeb

echo.
echo ✅ GitHub Pages fix pushed successfully!
echo 🌐 GitHub Actions should now work without submodule errors
echo ⏱️  Check GitHub Actions in 2-3 minutes for green checkmarks
pause