/**
 * @file nightmare_config.h
 * @brief Nightmare Edition - Configuration System
 * 
 * SISTEMA DE CONFIGURACIÓN NIGHTMARE
 * =================================
 * 
 * Sistema modular de activación de features experimentales mediante archivos.
 * Permite a los usuarios elegir qué características quieren activar.
 * 
 * FILOSOFÍA:
 * - Activación opcional y segura
 * - Testing modular sin romper el juego base
 * - Configuración granular por feature
 */

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace devilution {

/**
 * @brief Tipos de features nightmare disponibles
 */
enum class NightmareFeature {
	LIGHTING,           ///< Lighting atmosférico avanzado
	POST_DIABLO,        ///< Contenido post-Diablo
	NIGHTMARE_ITEMS,    ///< Items únicos temáticos
	AMBIENCE,           ///< Ambientación completa (NPCs, lugares)
	SECRET_LEVELS,      ///< Niveles secretos adicionales
	NIGHTMARE_QUESTS,   ///< Quests post-Diablo
	ENHANCED_AUDIO,     ///< Sistema de audio mejorado
	VISUAL_EFFECTS,     ///< Efectos visuales avanzados
	DEBUG_FEATURES      ///< Features de debug y testing
};

/**
 * @brief Estado de configuración nightmare
 */
struct NightmareConfig {
	bool initialized = false;
	std::unordered_map<NightmareFeature, bool> features;
	std::string configPath = "nightmare_config/";
	
	// Cache de archivos verificados
	std::unordered_map<std::string, bool> fileCache;
	uint32_t lastCheckTime = 0;
	uint32_t checkInterval = 5000; // Verificar cada 5 segundos
};

// Estado global
extern NightmareConfig g_nightmareConfig;

/**
 * @brief Inicializa el sistema de configuración nightmare
 */
void InitNightmareConfig();

/**
 * @brief Verifica si una feature está habilitada
 * @param feature Feature a verificar
 * @return true si está habilitada
 */
bool IsNightmareFeatureEnabled(NightmareFeature feature);

/**
 * @brief Verifica si un archivo de configuración existe
 * @param filename Nombre del archivo (ej: "enable_lighting")
 * @return true si el archivo existe
 */
bool IsNightmareFileEnabled(const std::string& filename);

/**
 * @brief Actualiza el estado de las features (verifica archivos)
 */
void UpdateNightmareConfig();

/**
 * @brief Habilita una feature programáticamente
 * @param feature Feature a habilitar
 * @param enabled Estado a establecer
 */
void SetNightmareFeature(NightmareFeature feature, bool enabled);

/**
 * @brief Crea archivos de configuración por defecto
 */
void CreateDefaultNightmareConfig();

/**
 * @brief Obtiene el nombre del archivo para una feature
 * @param feature Feature
 * @return Nombre del archivo correspondiente
 */
std::string GetNightmareFeatureFilename(NightmareFeature feature);

/**
 * @brief Limpia el sistema de configuración
 */
void CleanupNightmareConfig();

// Macros de conveniencia para verificar features
#define NIGHTMARE_LIGHTING_ENABLED() IsNightmareFeatureEnabled(NightmareFeature::LIGHTING)
#define NIGHTMARE_POST_DIABLO_ENABLED() IsNightmareFeatureEnabled(NightmareFeature::POST_DIABLO)
#define NIGHTMARE_ITEMS_ENABLED() IsNightmareFeatureEnabled(NightmareFeature::NIGHTMARE_ITEMS)
#define NIGHTMARE_AMBIENCE_ENABLED() IsNightmareFeatureEnabled(NightmareFeature::AMBIENCE)
#define NIGHTMARE_SECRET_LEVELS_ENABLED() IsNightmareFeatureEnabled(NightmareFeature::SECRET_LEVELS)
#define NIGHTMARE_QUESTS_ENABLED() IsNightmareFeatureEnabled(NightmareFeature::NIGHTMARE_QUESTS)
#define NIGHTMARE_ENHANCED_AUDIO_ENABLED() IsNightmareFeatureEnabled(NightmareFeature::ENHANCED_AUDIO)
#define NIGHTMARE_VISUAL_EFFECTS_ENABLED() IsNightmareFeatureEnabled(NightmareFeature::VISUAL_EFFECTS)
#define NIGHTMARE_DEBUG_ENABLED() IsNightmareFeatureEnabled(NightmareFeature::DEBUG_FEATURES)

} // namespace devilution