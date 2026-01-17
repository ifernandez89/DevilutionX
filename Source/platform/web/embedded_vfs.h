/**
 * @file embedded_vfs.h
 * @brief Embedded filesystem implementation for WebAssembly builds
 * 
 * This implementation provides access to assets embedded directly
 * in the WebAssembly binary, eliminating the need for separate
 * file downloads and providing instant loading.
 */

#pragma once

#include "abstractions/virtual_filesystem.h"
#include <unordered_map>

namespace devilution {

/**
 * @brief Asset data structure for embedded files
 */
struct EmbeddedAsset {
    const uint8_t* data;
    size_t size;
    
    EmbeddedAsset(const uint8_t* data, size_t size) : data(data), size(size) {}
};

/**
 * @brief Embedded filesystem implementation for WebAssembly
 * 
 * This VFS implementation provides access to assets that are embedded
 * directly in the WebAssembly binary. Assets are registered at compile
 * time and accessed through a hash map for O(1) lookup.
 * 
 * Features:
 * - Instant asset loading (no network requests)
 * - Memory-efficient (assets are in read-only memory)
 * - Deterministic behavior across all platforms
 * - No filesystem dependencies
 */
class EmbeddedVFS : public VirtualFileSystem {
private:
    std::unordered_map<std::string, EmbeddedAsset> assets;
    std::unordered_map<std::string, std::vector<uint8_t>> memoryFiles; // For temporary saves

public:
    EmbeddedVFS();

    /**
     * @brief Register an embedded asset
     * @param path Virtual path for the asset
     * @param data Pointer to embedded data
     * @param size Size of the embedded data
     */
    void RegisterAsset(const std::string& path, const uint8_t* data, size_t size);

    /**
     * @brief Register multiple assets from a manifest
     * This is typically called from auto-generated code
     */
    void RegisterAssetsFromManifest();

    // VirtualFileSystem interface implementation
    std::vector<uint8_t> LoadFile(const std::string& path) override;
    bool FileExists(const std::string& path) override;
    size_t GetFileSize(const std::string& path) override;
    bool SaveFile(const std::string& path, const std::vector<uint8_t>& data) override;
    bool CreateDirectory(const std::string& path) override;
    std::vector<std::string> ListFiles(const std::string& path) override;
    std::string GetImplementationType() const override;

    /**
     * @brief Get statistics about embedded assets
     * @return String with asset count and total size
     */
    std::string GetAssetStats() const;

private:
    /**
     * @brief Normalize path for consistent lookup
     * @param path Input path
     * @return Normalized path
     */
    std::string NormalizePath(const std::string& path) const;

    /**
     * @brief Check if path is in memory files (temporary saves)
     * @param path Path to check
     * @return true if it's a memory file
     */
    bool IsMemoryFile(const std::string& path) const;
};

} // namespace devilution