/**
 * @file nightmare_config.cpp
 * @brief Nightmare Edition - Configuration System Implementation
 * 
 * IMPLEMENTACIÓN DEL SISTEMA DE CONFIGURACIÓN
 * ==========================================
 * 
 * Sistema que permite activar/desactivar features mediante archivos.
 */

#include "nightmare_config.h"

#include <filesystem>
#include <fstream>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "utils/log.hpp"

namespace devilution {

// Estado global del sistema
NightmareConfig g_nightmareConfig;

// Mapeo de features a nombres de archivo
namespace {
	const std::unordered_map<NightmareFeature, std::string> FeatureFilenames = {
		{ NightmareFeature::LIGHTING, "enable_nightmare_lighting" },
		{ NightmareFeature::POST_DIABLO, "enable_post_diablo" },
		{ NightmareFeature::NIGHTMARE_ITEMS, "enable_nightmare_items" },
		{ NightmareFeature::AMBIENCE, "enable_nightmare_ambience" },
		{ NightmareFeature::SECRET_LEVELS, "enable_secret_levels" },
		{ NightmareFeature::NIGHTMARE_QUESTS, "enable_nightmare_quests" },
		{ NightmareFeature::ENHANCED_AUDIO, "enable_enhanced_audio" },
		{ NightmareFeature::VISUAL_EFFECTS, "enable_visual_effects" },
		{ NightmareFeature::DEBUG_FEATURES, "enable_debug_features" }
	};
}

void InitNightmareConfig()
{
	LogVerbose("🌙 Initializing Nightmare Configuration System");
	
	g_nightmareConfig.initialized = true;
	g_nightmareConfig.lastCheckTime = SDL_GetTicks();
	
	// Crear directorio de configuración si no existe
	std::filesystem::create_directories(g_nightmareConfig.configPath);
	
	// Inicializar todas las features como HABILITADAS por defecto
	for (const auto& [feature, filename] : FeatureFilenames) {
		g_nightmareConfig.features[feature] = true; // HABILITADO POR DEFECTO
	}
	
	// Verificar estado inicial
	UpdateNightmareConfig();
	
	// Crear archivos de configuración por defecto si no existen
	CreateDefaultNightmareConfig();
	
	LogVerbose("🌙 Nightmare Configuration System initialized");
	LogVerbose("📁 Config path: {}", g_nightmareConfig.configPath);
	LogVerbose("🔥 All features enabled by default");
}

bool IsNightmareFeatureEnabled(NightmareFeature feature)
{
	if (!g_nightmareConfig.initialized) {
		return false;
	}
	
	auto it = g_nightmareConfig.features.find(feature);
	return it != g_nightmareConfig.features.end() && it->second;
}

bool IsNightmareFileEnabled(const std::string& filename)
{
	if (!g_nightmareConfig.initialized) {
		return false;
	}
	
	// Verificar cache primero
	auto cacheIt = g_nightmareConfig.fileCache.find(filename);
	if (cacheIt != g_nightmareConfig.fileCache.end()) {
		return cacheIt->second;
	}
	
	// Verificar si el archivo existe
	std::string fullPath = g_nightmareConfig.configPath + filename;
	bool exists = std::filesystem::exists(fullPath);
	
	// Actualizar cache
	g_nightmareConfig.fileCache[filename] = exists;
	
	return exists;
}

void UpdateNightmareConfig()
{
	if (!g_nightmareConfig.initialized) {
		return;
	}
	
	uint32_t currentTime = SDL_GetTicks();
	
	// Solo verificar cada checkInterval ms para performance
	if (currentTime - g_nightmareConfig.lastCheckTime < g_nightmareConfig.checkInterval) {
		return;
	}
	
	g_nightmareConfig.lastCheckTime = currentTime;
	
	// Limpiar cache para forzar re-verificación
	g_nightmareConfig.fileCache.clear();
	
	// Verificar cada feature
	for (const auto& [feature, filename] : FeatureFilenames) {
		bool enabled = IsNightmareFileEnabled(filename);
		
		// Solo log si cambió el estado
		if (g_nightmareConfig.features[feature] != enabled) {
			g_nightmareConfig.features[feature] = enabled;
			LogVerbose("🌙 Feature {} {}: {}", 
				static_cast<int>(feature), 
				filename, 
				enabled ? "ENABLED" : "DISABLED");
		}
	}
}

void SetNightmareFeature(NightmareFeature feature, bool enabled)
{
	if (!g_nightmareConfig.initialized) {
		return;
	}
	
	g_nightmareConfig.features[feature] = enabled;
	
	LogVerbose("🌙 Feature {} set to: {}", 
		static_cast<int>(feature), 
		enabled ? "ENABLED" : "DISABLED");
}

std::string GetNightmareFeatureFilename(NightmareFeature feature)
{
	auto it = FeatureFilenames.find(feature);
	return it != FeatureFilenames.end() ? it->second : "";
}

void CreateDefaultNightmareConfig()
{
	// Crear archivo README explicativo
	std::string readmePath = g_nightmareConfig.configPath + "README.txt";
	if (!std::filesystem::exists(readmePath)) {
		std::ofstream readme(readmePath);
		if (readme.is_open()) {
			readme << "NIGHTMARE EDITION - CONFIGURATION FILES\n";
			readme << "=======================================\n\n";
			readme << "This directory contains configuration files for Nightmare Edition features.\n";
			readme << "Create empty files with these names to enable features:\n\n";
			
			for (const auto& [feature, filename] : FeatureFilenames) {
				readme << "  " << filename << " - Enable feature " << static_cast<int>(feature) << "\n";
			}
			
			readme << "\nFeature descriptions:\n";
			readme << "  enable_nightmare_lighting - Atmospheric lighting effects\n";
			readme << "  enable_post_diablo - Content available after defeating Diablo\n";
			readme << "  enable_nightmare_items - Unique nightmare-themed items\n";
			readme << "  enable_nightmare_ambience - Renamed NPCs, places, darker atmosphere\n";
			readme << "  enable_secret_levels - Additional secret levels (Level 17: The Void)\n";
			readme << "  enable_nightmare_quests - Post-Diablo quests and challenges\n";
			readme << "  enable_enhanced_audio - Advanced audio effects and atmosphere\n";
			readme << "  enable_visual_effects - Enhanced visual effects and shaders\n";
			readme << "  enable_debug_features - Debug and testing features\n\n";
			readme << "Example: To enable atmospheric lighting, create an empty file named 'enable_nightmare_lighting'\n";
			readme << "The file content doesn't matter - only its existence is checked.\n";
			
			readme.close();
			LogVerbose("📄 Created nightmare config README");
		}
	}
	
	// Crear archivo de ejemplo para lighting (ya que está implementado)
	std::string lightingPath = g_nightmareConfig.configPath + "enable_nightmare_lighting";
	if (!std::filesystem::exists(lightingPath)) {
		std::ofstream lightingFile(lightingPath);
		if (lightingFile.is_open()) {
			lightingFile << "# Nightmare Edition - Atmospheric Lighting\n";
			lightingFile << "# This file enables atmospheric lighting effects\n";
			lightingFile << "# Delete this file to disable the feature\n";
			lightingFile.close();
			LogVerbose("🔥 Created default lighting config file");
		}
	}
}

void CleanupNightmareConfig()
{
	LogVerbose("🌙 Cleaning up Nightmare Configuration System");
	
	g_nightmareConfig.initialized = false;
	g_nightmareConfig.features.clear();
	g_nightmareConfig.fileCache.clear();
}

} // namespace devilution