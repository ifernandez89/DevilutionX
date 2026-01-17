/**
 * @file vfs_wrappers.h
 * @brief VFS-compatible wrappers for existing file loading functions
 * 
 * This file provides drop-in replacements for existing file loading
 * functions that use the VFS abstraction layer instead of direct
 * filesystem access.
 */

#pragma once

#include <memory>
#include <array>
#include <expected.hpp>

#include "abstractions/virtual_filesystem.h"

namespace devilution {

/**
 * @brief VFS-compatible version of LoadFileInMem
 * @tparam T Type of data to load (e.g., uint8_t, uint16_t)
 * @param path Relative path to the file
 * @return Unique pointer to loaded data, or nullptr if failed
 */
template<typename T = uint8_t>
std::unique_ptr<T[]> LoadFileInMemVFS(const std::string& path)
{
    std::vector<uint8_t> data = LoadVirtualFile(path);
    if (data.empty()) {
        return nullptr;
    }
    
    // Ensure data size is compatible with target type
    const size_t elementCount = data.size() / sizeof(T);
    if (elementCount == 0) {
        return nullptr;
    }
    
    auto result = std::make_unique<T[]>(elementCount);
    std::memcpy(result.get(), data.data(), elementCount * sizeof(T));
    
    return result;
}

/**
 * @brief VFS-compatible version of LoadFileInMemWithStatus
 * @tparam T Type of data to load
 * @param path Relative path to the file
 * @return Expected with loaded data or error message
 */
template<typename T = uint8_t>
tl::expected<std::unique_ptr<T[]>, std::string> LoadFileInMemWithStatusVFS(const std::string& path)
{
    if (!VirtualFileExists(path)) {
        return tl::unexpected(std::string("File not found: ") + path);
    }
    
    std::vector<uint8_t> data = LoadVirtualFile(path);
    if (data.empty()) {
        return tl::unexpected(std::string("Failed to load file: ") + path);
    }
    
    const size_t elementCount = data.size() / sizeof(T);
    if (elementCount == 0) {
        return tl::unexpected(std::string("Invalid file size for type: ") + path);
    }
    
    auto result = std::make_unique<T[]>(elementCount);
    std::memcpy(result.get(), data.data(), elementCount * sizeof(T));
    
    return result;
}

/**
 * @brief VFS-compatible version of LoadFileInMem for fixed-size arrays
 * @tparam T Type of array elements
 * @tparam N Size of the array
 * @param path Relative path to the file
 * @param array Reference to array to fill
 * @return true if successful, false otherwise
 */
template<typename T, size_t N>
bool LoadFileInMemVFS(const std::string& path, std::array<T, N>& array)
{
    std::vector<uint8_t> data = LoadVirtualFile(path);
    if (data.size() != sizeof(array)) {
        return false;
    }
    
    std::memcpy(array.data(), data.data(), sizeof(array));
    return true;
}

/**
 * @brief VFS-compatible version of LoadFileInMemWithStatus for fixed-size arrays
 * @tparam T Type of array elements
 * @tparam N Size of the array
 * @param path Relative path to the file
 * @param array Reference to array to fill
 * @return Expected with success or error message
 */
template<typename T, size_t N>
tl::expected<void, std::string> LoadFileInMemWithStatusVFS(const std::string& path, std::array<T, N>& array)
{
    if (!VirtualFileExists(path)) {
        return tl::unexpected(std::string("File not found: ") + path);
    }
    
    std::vector<uint8_t> data = LoadVirtualFile(path);
    if (data.size() != sizeof(array)) {
        return tl::unexpected(std::string("File size mismatch for: ") + path);
    }
    
    std::memcpy(array.data(), data.data(), sizeof(array));
    return {};
}

/**
 * @brief Initialize VFS with appropriate implementation based on build target
 * This should be called early in the application startup
 */
void InitializeVFSForPlatform();

/**
 * @brief Shutdown VFS and cleanup resources
 */
void ShutdownVFS();

} // namespace devilution