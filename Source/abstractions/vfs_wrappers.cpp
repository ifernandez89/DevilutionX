/**
 * @file vfs_wrappers.cpp
 * @brief Implementation of VFS wrapper functions
 */

#include "vfs_wrappers.h"

#include "platform/native/native_vfs.h"
#include "utils/log.hpp"

#ifdef __EMSCRIPTEN__
#include "platform/web/embedded_vfs.h"
#endif

namespace devilution {

void InitializeVFSForPlatform()
{
#ifdef __EMSCRIPTEN__
    // WebAssembly build - use embedded VFS
    LogInfo("Initializing EmbeddedVFS for WebAssembly build");
    InitializeVFS(std::make_unique<EmbeddedVFS>());
#else
    // Desktop build - use native VFS
    LogInfo("Initializing NativeVFS for desktop build");
    InitializeVFS(std::make_unique<NativeVFS>());
#endif
    
    LogInfo("VFS initialized: {}", GetVFS().GetImplementationType());
}

void ShutdownVFS()
{
    if (g_virtualFileSystem) {
        LogInfo("Shutting down VFS: {}", g_virtualFileSystem->GetImplementationType());
        g_virtualFileSystem.reset();
    }
}

} // namespace devilution