/**
 * @file virtual_filesystem.cpp
 * @brief Implementation of VFS convenience functions
 */

#include "virtual_filesystem.h"

#include <stdexcept>

namespace devilution {

std::unique_ptr<VirtualFileSystem> g_virtualFileSystem;

void InitializeVFS(std::unique_ptr<VirtualFileSystem> vfs)
{
    g_virtualFileSystem = std::move(vfs);
}

VirtualFileSystem& GetVFS()
{
    if (!g_virtualFileSystem) {
        throw std::runtime_error("VFS not initialized! Call InitializeVFS() first.");
    }
    return *g_virtualFileSystem;
}

std::vector<uint8_t> LoadVirtualFile(const std::string& path)
{
    return GetVFS().LoadFile(path);
}

bool VirtualFileExists(const std::string& path)
{
    return GetVFS().FileExists(path);
}

bool SaveVirtualFile(const std::string& path, const std::vector<uint8_t>& data)
{
    return GetVFS().SaveFile(path, data);
}

} // namespace devilution