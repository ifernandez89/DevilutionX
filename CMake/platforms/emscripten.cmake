set(BUILD_TESTING OFF)
set(BUILD_ASSETS_MPQ OFF)
set(DISABLE_ZERO_TIER ON)
set(DISABLE_TCP ON)
set(DEVILUTIONX_SYSTEM_SDL_AUDIOLIB OFF)
set(DEVILUTIONX_SYSTEM_LIBSODIUM OFF)
set(DEVILUTIONX_SYSTEM_LIBFMT OFF)
set(NOEXIT ON)
set(ASAN OFF CACHE BOOL "Enable address sanitizer" FORCE)
set(UBSAN OFF CACHE BOOL "Enable undefined behaviour sanitizer" FORCE)
set(TSAN OFF CACHE BOOL "Enable thread sanitizer" FORCE)

# Emscripten ports do have a bzip2 but it fails to link with this error:
#   warning: _BZ2_bzDecompress may need to be added to EXPORTED_FUNCTIONS if it arrives from a system library
#   error: undefined symbol: BZ2_bzDecompressEnd (referenced by top-level compiled C/C++ code)
set(DEVILUTIONX_SYSTEM_BZIP2 OFF)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Packaging/emscripten/index.html" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Packaging/emscripten/file-manager.js" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
