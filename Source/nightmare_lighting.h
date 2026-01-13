/**
 * @file nightmare_lighting.h
 * @brief Nightmare Edition - Atmospheric Lighting System
 * 
 * SISTEMA DE ILUMINACIÓN ATMOSFÉRICA NIGHTMARE
 * ============================================
 * 
 * Sistema avanzado de iluminación que agrega:
 * - Antorchas parpadeantes con variación orgánica
 * - Efectos de sombra dinámica
 * - Iluminación de hechizos mejorada
 * - Niebla iluminada
 * 
 * FILOSOFÍA NIGHTMARE:
 * - Parpadeo sutil, no obvio (±1-2 radius)
 * - Timing orgánico, no mecánico
 * - Atmósfera tenebrosa, no brillante
 * - Performance O(1), sin impacto en FPS
 */

#pragma once

#include <cstdint>
#include <array>

namespace devilution {

/**
 * @brief Tipos de fuentes de luz atmosféricas
 */
enum class AtmosphericLightType {
	TORCH,          ///< Antorcha (parpadeo medio)
	CANDLE,         ///< Vela (parpadeo rápido)
	FIRE,           ///< Fuego (parpadeo intenso)
	MAGIC,          ///< Magia (pulso suave)
	NONE            ///< Sin efecto atmosférico
};

/**
 * @brief Estado de una luz atmosférica
 */
struct AtmosphericLight {
	int lightId = -1;                           ///< ID de la luz en el sistema
	AtmosphericLightType type = AtmosphericLightType::NONE;
	uint8_t baseRadius = 0;                     ///< Radio base original
	int8_t currentFlicker = 0;                  ///< Variación actual (-2 a +2)
	uint32_t lastFlickerTime = 0;               ///< Último tiempo de cambio
	uint32_t flickerDelay = 0;                  ///< Delay hasta próximo cambio
	bool enabled = true;                        ///< Luz habilitada
};

/**
 * @brief Configuración del sistema de iluminación atmosférica
 */
struct NightmareLightingConfig {
	bool enabled = true;                        ///< Sistema habilitado
	bool torchFlicker = true;                   ///< Parpadeo de antorchas
	bool candleFlicker = true;                  ///< Parpadeo de velas
	bool fireFlicker = true;                    ///< Parpadeo de fuego
	bool magicPulse = true;                     ///< Pulso de magia
	float flickerIntensity = 1.0f;              ///< Intensidad global (0.0-2.0)
	uint32_t minFlickerDelay = 100;             ///< Delay mínimo entre cambios (ms)
	uint32_t maxFlickerDelay = 400;             ///< Delay máximo entre cambios (ms)
};

/**
 * @brief Estado global del sistema de iluminación atmosférica
 */
struct NightmareLightingState {
	bool initialized = false;
	NightmareLightingConfig config;
	std::array<AtmosphericLight, 32> lights;    ///< Hasta 32 luces atmosféricas
	int activeLightCount = 0;
	uint32_t lastUpdateTime = 0;
};

// Estado global
extern NightmareLightingState g_nightmareLighting;

/**
 * @brief Inicializa el sistema de iluminación atmosférica
 */
void InitNightmareLighting();

/**
 * @brief Actualiza todas las luces atmosféricas
 */
void UpdateNightmareLighting();

/**
 * @brief Registra una luz para efectos atmosféricos
 * @param lightId ID de la luz en el sistema
 * @param type Tipo de luz atmosférica
 * @param baseRadius Radio base de la luz
 * @return Índice en el array de luces atmosféricas, o -1 si falla
 */
int RegisterAtmosphericLight(int lightId, AtmosphericLightType type, uint8_t baseRadius);

/**
 * @brief Desregistra una luz atmosférica
 * @param atmosphericIndex Índice en el array de luces atmosféricas
 */
void UnregisterAtmosphericLight(int atmosphericIndex);

/**
 * @brief Habilita/deshabilita el sistema completo
 * @param enabled true para habilitar
 */
void SetNightmareLightingEnabled(bool enabled);

/**
 * @brief Habilita/deshabilita parpadeo de antorchas
 * @param enabled true para habilitar
 */
void SetTorchFlickerEnabled(bool enabled);

/**
 * @brief Establece la intensidad global del parpadeo
 * @param intensity Intensidad (0.0 = sin efecto, 1.0 = normal, 2.0 = intenso)
 */
void SetFlickerIntensity(float intensity);

/**
 * @brief Limpia el sistema de iluminación atmosférica
 */
void CleanupNightmareLighting();

} // namespace devilution
