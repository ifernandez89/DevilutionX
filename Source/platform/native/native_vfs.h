/**
 * @file native_vfs.h
 * @brief Native filesystem implementation for desktop builds
 * 
 * This implementation provides direct access to the native filesystem,
 * maintaining full compatibility with existing DevilutionX behavior.
 */

#pragma once

#include "abstractions/virtual_filesystem.h"

namespace devilution {

/**
 * @brief Native filesystem implementation
 * 
 * This VFS implementation provides direct access to the native filesystem.
 * It's used for desktop builds to maintain existing behavior while
 * providing the abstraction layer needed for WebAssembly compatibility.
 */
class NativeVFS : public VirtualFileSystem {
private:
    std::string basePath;

public:
    /**
     * @brief Constructor
     * @param basePath Base directory for all file operations (usually game directory)
     */
    explicit NativeVFS(const std::string& basePath = "");

    // VirtualFileSystem interface implementation
    std::vector<uint8_t> LoadFile(const std::string& path) override;
    bool FileExists(const std::string& path) override;
    size_t GetFileSize(const std::string& path) override;
    bool SaveFile(const std::string& path, const std::vector<uint8_t>& data) override;
    bool CreateDirectory(const std::string& path) override;
    std::vector<std::string> ListFiles(const std::string& path) override;
    std::string GetImplementationType() const override;

private:
    /**
     * @brief Convert relative path to absolute path
     * @param relativePath Relative path from VFS
     * @return Absolute path for native filesystem
     */
    std::string GetAbsolutePath(const std::string& relativePath) const;

    /**
     * @brief Normalize path separators for current platform
     * @param path Path to normalize
     * @return Normalized path
     */
    std::string NormalizePath(const std::string& path) const;
};

} // namespace devilution