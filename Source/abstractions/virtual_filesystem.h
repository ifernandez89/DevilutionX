/**
 * @file virtual_filesystem.h
 * @brief Virtual Filesystem abstraction for WebAssembly compatibility
 * 
 * This abstraction layer allows the game to work with different filesystem
 * implementations:
 * - Native filesystem for desktop builds
 * - Embedded assets for WebAssembly builds
 * - Memory-based filesystem for testing
 * 
 * DESIGN PRINCIPLES:
 * - Simple interface, complex implementations
 * - No filesystem paths leak to game logic
 * - Deterministic behavior across platforms
 * - Efficient asset loading for Web
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace devilution {

/**
 * @brief Abstract interface for filesystem operations
 * 
 * This interface provides a platform-agnostic way to access files.
 * Different implementations can provide:
 * - Native file access (desktop)
 * - Embedded asset access (WebAssembly)
 * - Memory-based access (testing)
 */
class VirtualFileSystem {
public:
    virtual ~VirtualFileSystem() = default;

    /**
     * @brief Load a file into memory
     * @param path Relative path to the file (e.g., "levels/l1data/banner1.dun")
     * @return File contents as byte vector, or empty if file not found
     */
    virtual std::vector<uint8_t> LoadFile(const std::string& path) = 0;

    /**
     * @brief Check if a file exists
     * @param path Relative path to the file
     * @return true if file exists, false otherwise
     */
    virtual bool FileExists(const std::string& path) = 0;

    /**
     * @brief Get file size without loading it
     * @param path Relative path to the file
     * @return File size in bytes, or 0 if file not found
     */
    virtual size_t GetFileSize(const std::string& path) = 0;

    /**
     * @brief Save data to a file (for temporary/session data)
     * @param path Relative path where to save
     * @param data Data to save
     * @return true if successful, false otherwise
     * 
     * NOTE: In WebAssembly builds, this may save to memory only
     */
    virtual bool SaveFile(const std::string& path, const std::vector<uint8_t>& data) = 0;

    /**
     * @brief Create a directory (for temporary/session data)
     * @param path Directory path to create
     * @return true if successful or already exists, false otherwise
     */
    virtual bool CreateDirectory(const std::string& path) = 0;

    /**
     * @brief List files in a directory
     * @param path Directory path
     * @return List of filenames in the directory
     */
    virtual std::vector<std::string> ListFiles(const std::string& path) = 0;

    /**
     * @brief Get the implementation type for debugging
     * @return String describing the VFS implementation
     */
    virtual std::string GetImplementationType() const = 0;
};

/**
 * @brief Global VFS instance
 * 
 * This is initialized at startup with the appropriate implementation
 * based on the target platform.
 */
extern std::unique_ptr<VirtualFileSystem> g_virtualFileSystem;

/**
 * @brief Initialize the VFS with the appropriate implementation
 * @param vfs VFS implementation to use
 */
void InitializeVFS(std::unique_ptr<VirtualFileSystem> vfs);

/**
 * @brief Get the current VFS instance
 * @return Reference to the current VFS
 */
VirtualFileSystem& GetVFS();

/**
 * @brief Convenience function to load a file
 * @param path Relative file path
 * @return File contents or empty vector if not found
 */
std::vector<uint8_t> LoadVirtualFile(const std::string& path);

/**
 * @brief Convenience function to check if file exists
 * @param path Relative file path
 * @return true if file exists
 */
bool VirtualFileExists(const std::string& path);

/**
 * @brief Convenience function to save a file
 * @param path Relative file path
 * @param data Data to save
 * @return true if successful
 */
bool SaveVirtualFile(const std::string& path, const std::vector<uint8_t>& data);

} // namespace devilution