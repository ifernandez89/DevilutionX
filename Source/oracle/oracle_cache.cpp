/**
 * @file oracle_cache.cpp
 * 
 * 🔮 EL ORÁCULO - Implementación del Sistema de Cache
 */

#include "oracle_cache.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <SDL.h>

#include "utils/log.hpp"
#include "utils/paths.h"
#include "utils/str_cat.hpp"

namespace devilution {

// ============================================================================
// 🔮 ESTADO GLOBAL
// ============================================================================

std::unordered_map<std::string, CacheEntry> OracleCache::cache;

// ============================================================================
// 🔮 IMPLEMENTACIÓN
// ============================================================================

void OracleCache::Initialize()
{
	Log("Oracle Cache: Initializing...");
	
	cache.clear();
	LoadFromDisk();
	
	Log("Oracle Cache: Initialized with {} entries", cache.size());
}

void OracleCache::SaveToDisk()
{
	// Obtener ruta del archivo de cache
	std::string cachePath = StrCat(paths::PrefPath(), "/", CACHE_FILE);
	
	std::ofstream file(cachePath, std::ios::binary);
	if (!file.is_open()) {
		Log("Oracle Cache: Failed to open cache file for writing: {}", cachePath);
		return;
	}
	
	// Escribir número de entradas
	uint32_t entryCount = static_cast<uint32_t>(cache.size());
	file.write(reinterpret_cast<const char*>(&entryCount), sizeof(entryCount));
	
	// Escribir cada entrada
	for (const auto& [key, entry] : cache) {
		// Escribir key length y key
		uint32_t keyLen = static_cast<uint32_t>(key.length());
		file.write(reinterpret_cast<const char*>(&keyLen), sizeof(keyLen));
		file.write(key.c_str(), keyLen);
		
		// Escribir response length y response
		uint32_t responseLen = static_cast<uint32_t>(entry.response.length());
		file.write(reinterpret_cast<const char*>(&responseLen), sizeof(responseLen));
		file.write(entry.response.c_str(), responseLen);
		
		// Escribir baseTextUsed length y baseTextUsed
		uint32_t baseTextLen = static_cast<uint32_t>(entry.baseTextUsed.length());
		file.write(reinterpret_cast<const char*>(&baseTextLen), sizeof(baseTextLen));
		file.write(entry.baseTextUsed.c_str(), baseTextLen);
		
		// Escribir metadata
		file.write(reinterpret_cast<const char*>(&entry.timestamp), sizeof(entry.timestamp));
		file.write(reinterpret_cast<const char*>(&entry.hitCount), sizeof(entry.hitCount));
		file.write(reinterpret_cast<const char*>(&entry.similarity), sizeof(entry.similarity));
	}
	
	file.close();
	
	Log("Oracle Cache: Saved {} entries to disk", entryCount);
}

void OracleCache::LoadFromDisk()
{
	// Obtener ruta del archivo de cache
	std::string cachePath = StrCat(paths::PrefPath(), "/", CACHE_FILE);
	
	std::ifstream file(cachePath, std::ios::binary);
	if (!file.is_open()) {
		Log("Oracle Cache: No cache file found (first run)");
		return;
	}
	
	// Leer número de entradas
	uint32_t entryCount = 0;
	file.read(reinterpret_cast<char*>(&entryCount), sizeof(entryCount));
	
	if (file.fail() || entryCount > 1000) {  // Sanity check
		Log("Oracle Cache: Invalid cache file");
		file.close();
		return;
	}
	
	// Leer cada entrada
	for (uint32_t i = 0; i < entryCount; ++i) {
		// Leer key
		uint32_t keyLen = 0;
		file.read(reinterpret_cast<char*>(&keyLen), sizeof(keyLen));
		if (file.fail() || keyLen > 1000) break;
		
		std::string key(keyLen, '\0');
		file.read(&key[0], keyLen);
		if (file.fail()) break;
		
		// Leer response
		uint32_t responseLen = 0;
		file.read(reinterpret_cast<char*>(&responseLen), sizeof(responseLen));
		if (file.fail() || responseLen > 2000) break;
		
		std::string response(responseLen, '\0');
		file.read(&response[0], responseLen);
		if (file.fail()) break;
		
		// Leer baseTextUsed
		uint32_t baseTextLen = 0;
		file.read(reinterpret_cast<char*>(&baseTextLen), sizeof(baseTextLen));
		if (file.fail() || baseTextLen > 2000) break;
		
		std::string baseTextUsed(baseTextLen, '\0');
		file.read(&baseTextUsed[0], baseTextLen);
		if (file.fail()) break;
		
		// Leer metadata
		CacheEntry entry;
		entry.response = response;
		entry.baseTextUsed = baseTextUsed;
		file.read(reinterpret_cast<char*>(&entry.timestamp), sizeof(entry.timestamp));
		file.read(reinterpret_cast<char*>(&entry.hitCount), sizeof(entry.hitCount));
		file.read(reinterpret_cast<char*>(&entry.similarity), sizeof(entry.similarity));
		
		if (file.fail()) break;
		
		cache[key] = entry;
	}
	
	file.close();
	
	Log("Oracle Cache: Loaded {} entries from disk", cache.size());
}

std::optional<std::string> OracleCache::GetResponse(
	const std::string& question,
	const std::string& baseText
)
{
	std::string key = GenerateCacheKey(question, baseText);
	
	auto it = cache.find(key);
	if (it != cache.end()) {
		// Cache hit!
		it->second.hitCount++;
		
#ifdef _DEBUG
		LogVerbose("Oracle Cache: HIT (key: {}, hits: {})", 
			key.substr(0, 16), it->second.hitCount);
#endif
		
		return it->second.response;
	}
	
#ifdef _DEBUG
	LogVerbose("Oracle Cache: MISS (key: {})", key.substr(0, 16));
#endif
	
	return std::nullopt;
}

void OracleCache::SaveResponse(
	const std::string& question,
	const std::string& response,
	const std::string& baseText,
	float similarity
)
{
	// Verificar límite de cache
	if (cache.size() >= MAX_CACHE_SIZE) {
		EvictLRU();
	}
	
	std::string key = GenerateCacheKey(question, baseText);
	
	CacheEntry entry;
	entry.response = response;
	entry.baseTextUsed = baseText;
	entry.timestamp = SDL_GetTicks();
	entry.hitCount = 0;
	entry.similarity = similarity;
	
	cache[key] = entry;
	
#ifdef _DEBUG
	LogVerbose("Oracle Cache: Saved response (key: {}, similarity: {:.2f})", 
		key.substr(0, 16), similarity);
#endif
	
	// Guardar a disco periódicamente (cada 10 entradas)
	if (cache.size() % 10 == 0) {
		SaveToDisk();
	}
}

void OracleCache::Clear()
{
	cache.clear();
	Log("Oracle Cache: Cleared");
}

void OracleCache::GetStats(size_t& totalEntries, uint32_t& totalHits, float& hitRate)
{
	totalEntries = cache.size();
	totalHits = 0;
	
	for (const auto& [key, entry] : cache) {
		totalHits += entry.hitCount;
	}
	
	// Calcular hit rate aproximado
	uint32_t totalQueries = totalHits + static_cast<uint32_t>(totalEntries);
	hitRate = (totalQueries > 0) ? 
		static_cast<float>(totalHits) / static_cast<float>(totalQueries) : 0.0f;
}

std::string OracleCache::GenerateCacheKey(
	const std::string& question,
	const std::string& baseText
)
{
	// Normalizar pregunta
	std::string normalized = NormalizeQuestion(question);
	
	// Si hay texto base, incluirlo en el key (primeras 50 chars)
	if (!baseText.empty()) {
		std::string baseNormalized = NormalizeQuestion(baseText);
		if (baseNormalized.length() > 50) {
			baseNormalized = baseNormalized.substr(0, 50);
		}
		normalized += "|" + baseNormalized;
	}
	
	// Hash simple (para mantener key corto)
	// En producción, usar hash real (std::hash, etc.)
	return normalized;
}

void OracleCache::EvictLRU()
{
	if (cache.empty()) {
		return;
	}
	
	// Encontrar entrada con menor hitCount y timestamp más antiguo
	auto lruIt = cache.begin();
	uint32_t minScore = lruIt->second.hitCount * 1000 + 
		(SDL_GetTicks() - lruIt->second.timestamp) / 1000;
	
	for (auto it = cache.begin(); it != cache.end(); ++it) {
		uint32_t score = it->second.hitCount * 1000 + 
			(SDL_GetTicks() - it->second.timestamp) / 1000;
		
		if (score < minScore) {
			minScore = score;
			lruIt = it;
		}
	}
	
	Log("Oracle Cache: Evicting LRU entry (hits: {})", lruIt->second.hitCount);
	cache.erase(lruIt);
}

std::string OracleCache::NormalizeQuestion(const std::string& question)
{
	std::string normalized;
	normalized.reserve(question.length());
	
	// Convertir a lowercase, eliminar puntuación
	for (char c : question) {
		if (std::isalpha(static_cast<unsigned char>(c))) {
			normalized += std::tolower(static_cast<unsigned char>(c));
		} else if (std::isspace(static_cast<unsigned char>(c))) {
			if (!normalized.empty() && normalized.back() != ' ') {
				normalized += ' ';
			}
		}
	}
	
	// Trim trailing space
	if (!normalized.empty() && normalized.back() == ' ') {
		normalized.pop_back();
	}
	
	return normalized;
}

} // namespace devilution
