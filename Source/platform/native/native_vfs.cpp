/**
 * @file native_vfs.cpp
 * @brief Implementation of native filesystem VFS
 */

#include "native_vfs.h"

#include <fstream>
#include <filesystem>
#include <algorithm>

#include "utils/file_util.h"
#include "utils/log.hpp"

namespace devilution {

NativeVFS::NativeVFS(const std::string& basePath)
    : basePath(basePath)
{
    LogVerbose("NativeVFS initialized with base path: '{}'", basePath.empty() ? "." : basePath);
}

std::vector<uint8_t> NativeVFS::LoadFile(const std::string& path)
{
    const std::string absolutePath = GetAbsolutePath(path);
    
    std::ifstream file(absolutePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        LogVerbose("NativeVFS: Failed to open file '{}'", absolutePath);
        return {};
    }

    const std::streamsize size = file.tellg();
    if (size <= 0) {
        LogVerbose("NativeVFS: File '{}' is empty or invalid size", absolutePath);
        return {};
    }

    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        LogVerbose("NativeVFS: Failed to read file '{}'", absolutePath);
        return {};
    }

    LogVerbose("NativeVFS: Successfully loaded file '{}' ({} bytes)", absolutePath, size);
    return buffer;
}

bool NativeVFS::FileExists(const std::string& path)
{
    const std::string absolutePath = GetAbsolutePath(path);
    return devilution::FileExists(absolutePath.c_str());
}

size_t NativeVFS::GetFileSize(const std::string& path)
{
    const std::string absolutePath = GetAbsolutePath(path);
    
    std::uintmax_t size;
    if (!devilution::GetFileSize(absolutePath.c_str(), &size)) {
        return 0;
    }
    
    return static_cast<size_t>(size);
}

bool NativeVFS::SaveFile(const std::string& path, const std::vector<uint8_t>& data)
{
    const std::string absolutePath = GetAbsolutePath(path);
    
    // Create directory if it doesn't exist (simplified version without exceptions)
    const std::filesystem::path filePath(absolutePath);
    const std::filesystem::path directory = filePath.parent_path();
    
    if (!directory.empty() && !std::filesystem::exists(directory)) {
        std::error_code ec;
        std::filesystem::create_directories(directory, ec);
        if (ec) {
            LogError("NativeVFS: Failed to create directory for '{}'", absolutePath);
            return false;
        }
    }
    
    std::ofstream file(absolutePath, std::ios::binary);
    if (!file.is_open()) {
        LogError("NativeVFS: Failed to create file '{}'", absolutePath);
        return false;
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    if (!file.good()) {
        LogError("NativeVFS: Failed to write to file '{}'", absolutePath);
        return false;
    }

    LogVerbose("NativeVFS: Successfully saved file '{}' ({} bytes)", absolutePath, data.size());
    return true;
}

bool NativeVFS::CreateDirectory(const std::string& path)
{
    const std::string absolutePath = GetAbsolutePath(path);
    
    std::error_code ec;
    bool result = std::filesystem::create_directories(absolutePath, ec);
    if (ec) {
        LogError("NativeVFS: Failed to create directory '{}'", absolutePath);
        return false;
    }
    return result;
}

std::vector<std::string> NativeVFS::ListFiles(const std::string& path)
{
    const std::string absolutePath = GetAbsolutePath(path);
    std::vector<std::string> files;
    
    std::error_code ec;
    if (!std::filesystem::exists(absolutePath, ec) || !std::filesystem::is_directory(absolutePath, ec)) {
        return files;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(absolutePath, ec)) {
        if (ec) break;
        if (entry.is_regular_file(ec) && !ec) {
            files.push_back(entry.path().filename().string());
        }
    }
    
    if (ec) {
        LogError("NativeVFS: Failed to list files in '{}'", absolutePath);
    }
    
    return files;
}

std::string NativeVFS::GetImplementationType() const
{
    return "NativeVFS (Desktop Filesystem)";
}

std::string NativeVFS::GetAbsolutePath(const std::string& relativePath) const
{
    if (basePath.empty()) {
        return NormalizePath(relativePath);
    }
    
    return NormalizePath(basePath + "/" + relativePath);
}

std::string NativeVFS::NormalizePath(const std::string& path) const
{
    std::string normalized = path;
    
    // Convert backslashes to forward slashes for consistency
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    
    // On Windows, convert back to backslashes for native filesystem
#ifdef _WIN32
    std::replace(normalized.begin(), normalized.end(), '/', '\\');
#endif
    
    return normalized;
}

} // namespace devilution