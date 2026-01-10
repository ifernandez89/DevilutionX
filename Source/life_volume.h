/**
 * @file life_volume.h
 * 
 * 🎨 FASE D2 - VIDA Y VOLUMEN 🎨
 * 
 * Sistema para añadir "vida" y movimiento sutil a los sprites
 * sin afectar gameplay, colisiones o lógica del juego.
 */

#pragma once

#include <cstdint>
#include <array>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "engine/point.hpp"
// Forward declarations to avoid heavy includes
struct Player;
struct Monster;

namespace devilution {

// ============================================================================
// 🎯 D2.1 - MICRO-OFFSETS DE RENDER
// ============================================================================

/**
 * @brief Tipos de animación de vida para diferentes entidades
 */
enum class LifeAnimationType : uint8_t {
	None = 0,           // Sin animación
	IdleBreathing,      // Respiración sutil cuando está parado
	WalkingBobbing,     // Bobbing al caminar
	CombatTension,      // Tensión en combate
	DeathTwitching,     // Contracciones al morir
	SpellCasting,       // Energía al lanzar hechizos
	ItemInteraction,    // Movimiento al interactuar con items
	NPCIdle,           // Animación específica para NPCs
	MonsterBreathing,   // Respiración de monstruos
	EliteGlow          // Pulsación para monstruos élite
};

/**
 * @brief Configuración de micro-offsets para una entidad
 */
struct LifeOffsetConfig {
	LifeAnimationType type = LifeAnimationType::None;
	float intensity = 1.0f;        // Multiplicador de intensidad (0.0-2.0)
	float frequency = 1.0f;        // Multiplicador de frecuencia (0.5-3.0)
	uint32_t startTime = 0;        // Tiempo de inicio de la animación
	bool enabled = true;           // Si está habilitado
	
	// Parámetros específicos por tipo
	float breathingAmplitude = 0.5f;   // Amplitud de respiración (pixels)
	float bobbingAmplitude = 1.0f;     // Amplitud de bobbing (pixels)
	float tensionAmplitude = 0.3f;     // Amplitud de tensión (pixels)
};

/**
 * @brief Resultado de cálculo de micro-offset
 */
struct LifeOffset {
	Point offset = { 0, 0 };       // Offset en pixels (x, y)
	float alpha = 1.0f;            // Modificador de transparencia
	bool isActive = false;         // Si el offset está activo
};

// ============================================================================
// 🎯 D2.2 - FEEDBACK DIRECCIONAL DE LUZ
// ============================================================================

/**
 * @brief Configuración de feedback direccional de luz
 */
struct LightVolumeConfig {
	bool enabled = true;
	float intensity = 0.8f;        // Intensidad del efecto (0.0-2.0)
	float falloffRate = 1.2f;      // Velocidad de falloff
	float ambientBoost = 0.1f;     // Boost de luz ambiente
	float directionalBoost = 0.3f; // Boost direccional
	
	// Configuración por tipo de entidad
	float playerLightBoost = 1.0f;
	float monsterLightBoost = 0.8f;
	float npcLightBoost = 0.6f;
	float itemLightBoost = 0.4f;
};

/**
 * @brief Resultado de cálculo de volumen de luz
 */
struct LightVolumeResult {
	float lightMultiplier = 1.0f;  // Multiplicador de luz base
	float shadowIntensity = 0.0f;  // Intensidad de sombra (0.0-1.0)
	Point lightDirection = { 0, 0 }; // Dirección de luz principal
	bool hasVolumetricEffect = false; // Si tiene efecto volumétrico
};

// ============================================================================
// 🎯 ESTADO GLOBAL DEL SISTEMA
// ============================================================================

/**
 * @brief Estado global del sistema de vida y volumen
 */
struct LifeVolumeState {
	bool enabled = true;
	bool microOffsetsEnabled = true;
	bool lightVolumeEnabled = true;
	
	// Configuraciones globales
	LifeOffsetConfig defaultPlayerConfig;
	LifeOffsetConfig defaultMonsterConfig;
	LifeOffsetConfig defaultNPCConfig;
	LightVolumeConfig lightConfig;
	
	// Métricas de performance
	uint32_t activeOffsets = 0;
	uint32_t activeLightCalculations = 0;
	uint32_t totalCalculationsThisFrame = 0;
	
	// Límites de performance
	static constexpr uint32_t MAX_ACTIVE_OFFSETS = 64;
	static constexpr uint32_t MAX_LIGHT_CALCULATIONS = 32;
};

/**
 * @brief Estadísticas del sistema para debugging
 */
struct LifeVolumeStats {
	uint64_t totalOffsetsCalculated = 0;
	uint64_t totalLightCalculations = 0;
	uint32_t peakActiveOffsets = 0;
	uint32_t peakLightCalculations = 0;
	float averageCalculationTime = 0.0f;
	uint32_t framesProcessed = 0;
};

// ============================================================================
// 🎯 API PRINCIPAL
// ============================================================================

/**
 * @brief Inicializar el sistema de vida y volumen
 */
void InitLifeVolume();

/**
 * @brief Actualizar el sistema cada frame
 */
void UpdateLifeVolume();

/**
 * @brief Calcular micro-offset para un player
 */
LifeOffset CalculatePlayerLifeOffset(const Player &player);

/**
 * @brief Calcular micro-offset para un monster
 */
LifeOffset CalculateMonsterLifeOffset(const Monster &monster);

/**
 * @brief Calcular feedback de luz volumétrica para una posición
 */
LightVolumeResult CalculateLightVolume(Point position, int lightRadius);

/**
 * @brief Aplicar micro-offset a coordenadas de render
 */
Point ApplyLifeOffset(Point originalPos, const LifeOffset &offset);

/**
 * @brief Aplicar feedback de luz a un color
 */
uint8_t ApplyLightVolume(uint8_t originalColor, const LightVolumeResult &lightResult);

// ============================================================================
// 🎯 CONFIGURACIÓN Y PRESETS
// ============================================================================

/**
 * @brief Aplicar preset balanceado para vida y volumen
 */
void ApplyBalancedLifeVolumePreset();

/**
 * @brief Aplicar preset sutil (mínimo impacto visual)
 */
void ApplySubtleLifeVolumePreset();

/**
 * @brief Aplicar preset dramático (máximo impacto visual)
 */
void ApplyDramaticLifeVolumePreset();

/**
 * @brief Configurar animación específica para un tipo
 */
void ConfigureLifeAnimation(LifeAnimationType type, const LifeOffsetConfig &config);

/**
 * @brief Obtener configuración actual del sistema
 */
const LifeVolumeState &GetLifeVolumeState();

/**
 * @brief Obtener estadísticas del sistema
 */
const LifeVolumeStats &GetLifeVolumeStats();

// ============================================================================
// 🎯 DEBUGGING Y UTILIDADES
// ============================================================================

#ifdef _DEBUG
/**
 * @brief Habilitar/deshabilitar logging de debugging
 */
void SetLifeVolumeDebugLogging(bool enabled);

/**
 * @brief Forzar tipo de animación para testing
 */
void ForceLifeAnimationType(LifeAnimationType type);

/**
 * @brief Obtener información de debugging como string
 */
std::string GetLifeVolumeDebugInfo();
#endif

} // namespace devilution