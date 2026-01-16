/**
 * @file oracle_cache.h
 * 
 * 🔮 EL ORÁCULO - Sistema de Cache Persistente
 * 
 * Cachea respuestas exitosas del Oráculo para:
 * - Reducir latencia (respuesta instantánea)
 * - Reducir llamadas a Ollama (ahorro de CPU)
 * - Reutilizar variaciones exitosas
 * - Persistencia entre sesiones
 * 
 * ESTRATEGIA:
 * - Cache key: hash(pregunta + texto_base_usado)
 * - Hit rate esperado: 70-80%
 * - Persistencia: JSON en disco
 * - Límite: 100 entradas (LRU eviction)
 */

#pragma once

#include <string>
#include <optional>
#include <unordered_map>
#include <cstdint>

namespace devilution {

/**
 * @brief Entrada de cache
 */
struct CacheEntry {
	std::string response;            // Respuesta cacheada
	std::string baseTextUsed;        // Texto base que se usó
	uint32_t timestamp;              // Cuándo se cacheó
	uint32_t hitCount;               // Cuántas veces se reutilizó
	float similarity;                // Similitud con texto base
};

/**
 * @brief Sistema de Cache del Oráculo
 * 
 * Cache en memoria con persistencia a disco.
 */
class OracleCache {
private:
	static std::unordered_map<std::string, CacheEntry> cache;
	static constexpr size_t MAX_CACHE_SIZE = 100;
	static constexpr const char* CACHE_FILE = "oracle_cache.dat";
	
public:
	/**
	 * @brief Inicializa el cache (carga desde disco)
	 */
	static void Initialize();
	
	/**
	 * @brief Guarda el cache a disco
	 */
	static void SaveToDisk();
	
	/**
	 * @brief Carga el cache desde disco
	 */
	static void LoadFromDisk();
	
	/**
	 * @brief Obtiene una respuesta del cache
	 * 
	 * @param question Pregunta del jugador
	 * @param baseText Texto base usado (opcional, mejora precisión)
	 * @return Respuesta cacheada o nullopt si no existe
	 */
	static std::optional<std::string> GetResponse(
		const std::string& question,
		const std::string& baseText = ""
	);
	
	/**
	 * @brief Guarda una respuesta en el cache
	 * 
	 * @param question Pregunta del jugador
	 * @param response Respuesta generada
	 * @param baseText Texto base usado
	 * @param similarity Similitud con texto base
	 */
	static void SaveResponse(
		const std::string& question,
		const std::string& response,
		const std::string& baseText,
		float similarity
	);
	
	/**
	 * @brief Limpia el cache
	 */
	static void Clear();
	
	/**
	 * @brief Obtiene estadísticas del cache
	 * 
	 * @param totalEntries Total de entradas
	 * @param totalHits Total de hits
	 * @param hitRate Hit rate (0.0-1.0)
	 */
	static void GetStats(size_t& totalEntries, uint32_t& totalHits, float& hitRate);
	
private:
	/**
	 * @brief Genera cache key
	 * @param question Pregunta
	 * @param baseText Texto base (opcional)
	 * @return Cache key (hash)
	 */
	static std::string GenerateCacheKey(
		const std::string& question,
		const std::string& baseText
	);
	
	/**
	 * @brief Evict entrada menos usada (LRU)
	 */
	static void EvictLRU();
	
	/**
	 * @brief Normaliza pregunta para cache key
	 * @param question Pregunta
	 * @return Pregunta normalizada
	 */
	static std::string NormalizeQuestion(const std::string& question);
};

} // namespace devilution
