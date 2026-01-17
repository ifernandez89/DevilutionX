/**
 * @file embedded_vfs.cpp
 * @brief Implementation of embedded filesystem for WebAssembly
 */

#include "embedded_vfs.h"

#include <algorithm>
#include <sstream>

#include "utils/log.hpp"

namespace devilution {

EmbeddedVFS::EmbeddedVFS()
{
    LogVerbose("EmbeddedVFS initialized for WebAssembly");
    RegisterAssetsFromManifest();
}

void EmbeddedVFS::RegisterAsset(const std::string& path, const uint8_t* data, size_t size)
{
    const std::string normalizedPath = NormalizePath(path);
    assets.emplace(normalizedPath, EmbeddedAsset(data, size));
    LogVerbose("EmbeddedVFS: Registered asset '{}' ({} bytes)", normalizedPath, size);
}

void EmbeddedVFS::RegisterAssetsFromManifest()
{
    // Stub implementation for WebAssembly builds without asset manifest
    LogInfo("EmbeddedVFS: Asset manifest disabled - using stub implementation");
    // In a production build, this would register all embedded assets
    // For now, we just log that the system is ready
}

std::vector<uint8_t> EmbeddedVFS::LoadFile(const std::string& path)
{
    const std::string normalizedPath = NormalizePath(path);
    
    // Check memory files first (temporary saves)
    if (IsMemoryFile(normalizedPath)) {
        auto it = memoryFiles.find(normalizedPath);
        if (it != memoryFiles.end()) {
            LogVerbose("EmbeddedVFS: Loaded memory file '{}' ({} bytes)", normalizedPath, it->second.size());
            return it->second;
        }
    }
    
    // Check embedded assets
    auto it = assets.find(normalizedPath);
    if (it == assets.end()) {
        LogVerbose("EmbeddedVFS: Asset '{}' not found", normalizedPath);
        return {};
    }
    
    const EmbeddedAsset& asset = it->second;
    std::vector<uint8_t> data(asset.data, asset.data + asset.size);
    
    LogVerbose("EmbeddedVFS: Loaded embedded asset '{}' ({} bytes)", normalizedPath, asset.size);
    return data;
}

bool EmbeddedVFS::FileExists(const std::string& path)
{
    const std::string normalizedPath = NormalizePath(path);
    
    // Check memory files first
    if (memoryFiles.find(normalizedPath) != memoryFiles.end()) {
        return true;
    }
    
    // Check embedded assets
    return assets.find(normalizedPath) != assets.end();
}

size_t EmbeddedVFS::GetFileSize(const std::string& path)
{
    const std::string normalizedPath = NormalizePath(path);
    
    // Check memory files first
    auto memIt = memoryFiles.find(normalizedPath);
    if (memIt != memoryFiles.end()) {
        return memIt->second.size();
    }
    
    // Check embedded assets
    auto assetIt = assets.find(normalizedPath);
    if (assetIt != assets.end()) {
        return assetIt->second.size;
    }
    
    return 0;
}

bool EmbeddedVFS::SaveFile(const std::string& path, const std::vector<uint8_t>& data)
{
    const std::string normalizedPath = NormalizePath(path);
    
    // In WebAssembly, we save to memory only
    memoryFiles[normalizedPath] = data;
    
    LogVerbose("EmbeddedVFS: Saved memory file '{}' ({} bytes)", normalizedPath, data.size());
    return true;
}

bool EmbeddedVFS::CreateDirectory(const std::string& path)
{
    // In embedded VFS, directories don't need to be created explicitly
    // We just track them implicitly through file paths
    LogVerbose("EmbeddedVFS: Directory creation requested for '{}' (no-op)", path);
    return true;
}

std::vector<std::string> EmbeddedVFS::ListFiles(const std::string& path)
{
    const std::string normalizedPath = NormalizePath(path);
    std::vector<std::string> files;
    
    // List embedded assets
    for (const auto& [assetPath, asset] : assets) {
        if (assetPath.starts_with(normalizedPath)) {
            // Extract filename from path
            size_t lastSlash = assetPath.find_last_of('/');
            if (lastSlash != std::string::npos) {
                std::string filename = assetPath.substr(lastSlash + 1);
                files.push_back(filename);
            } else {
                files.push_back(assetPath);
            }
        }
    }
    
    // List memory files
    for (const auto& [memPath, data] : memoryFiles) {
        if (memPath.starts_with(normalizedPath)) {
            size_t lastSlash = memPath.find_last_of('/');
            if (lastSlash != std::string::npos) {
                std::string filename = memPath.substr(lastSlash + 1);
                files.push_back(filename);
            } else {
                files.push_back(memPath);
            }
        }
    }
    
    // Remove duplicates
    std::sort(files.begin(), files.end());
    files.erase(std::unique(files.begin(), files.end()), files.end());
    
    return files;
}

std::string EmbeddedVFS::GetImplementationType() const
{
    return "EmbeddedVFS (WebAssembly Assets)";
}

std::string EmbeddedVFS::GetAssetStats() const
{
    size_t totalSize = 0;
    for (const auto& [path, asset] : assets) {
        totalSize += asset.size;
    }
    
    std::ostringstream oss;
    oss << "EmbeddedVFS Stats: " << assets.size() << " assets, " 
        << totalSize << " bytes total, " << memoryFiles.size() << " memory files";
    return oss.str();
}

std::string EmbeddedVFS::NormalizePath(const std::string& path) const
{
    std::string normalized = path;
    
    // Convert backslashes to forward slashes
    std::replace(normalized.begin(), normalized.end(), '\\', '/');
    
    // Remove leading slash if present
    if (!normalized.empty() && normalized[0] == '/') {
        normalized = normalized.substr(1);
    }
    
    // Convert to lowercase for case-insensitive matching
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), ::tolower);
    
    return normalized;
}

bool EmbeddedVFS::IsMemoryFile(const std::string& path) const
{
    // Memory files are typically in debug_logs/ or other temporary directories
    return path.starts_with("debug_logs/") || 
           path.starts_with("temp/") || 
           path.ends_with(".log") ||
           path.ends_with(".tmp");
}

} // namespace devilution