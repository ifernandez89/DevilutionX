/**
 * @file virtual_filesystem.cpp
 * @brief Implementation of VFS convenience functions
 */

#include "virtual_filesystem.h"

#include <stdexcept>
#include <cstring>  // For memcpy

namespace devilution {

std::unique_ptr<VirtualFileSystem> g_virtualFileSystem;

void InitializeVFS(std::unique_ptr<VirtualFileSystem> vfs)
{
    g_virtualFileSystem = std::move(vfs);
}

VirtualFileSystem& GetVFS()
{
    if (!g_virtualFileSystem) {
        // Use a simple error handling instead of exceptions
        // This will be caught by the calling code
        static class NullVFS : public VirtualFileSystem {
        public:
            std::vector<uint8_t> LoadFile(const std::string&) override { return {}; }
            bool FileExists(const std::string&) override { return false; }
            size_t GetFileSize(const std::string&) override { return 0; }
            bool SaveFile(const std::string&, const std::vector<uint8_t>&) override { return false; }
            bool CreateDirectory(const std::string&) override { return false; }
            std::vector<std::string> ListFiles(const std::string&) override { return {}; }
            std::string GetImplementationType() const override { return "NullVFS (Error State)"; }
        } nullVFS;
        return nullVFS;
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