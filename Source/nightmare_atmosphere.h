/**
 * @file nightmare_atmosphere.h
 * @brief Nightmare Edition - Atmospheric Effects System
 * 
 * ATMÓSFERA NIGHTMARE SUTIL
 * ========================
 * 
 * Sistema de efectos atmosféricos que modifican sutilmente
 * la percepción visual sin que el jugador lo note conscientemente.
 * 
 * PRINCIPIOS NIGHTMARE:
 * - El jugador no sabe por qué se siente distinto, pero lo siente
 * - Desaturación leve (5-8%)
 * - Viento visual en sombras
 * - Respiración sutil en NPCs
 * 
 * FILOSOFÍA:
 * "Incomodar suavemente, eso es Diablo"
 */

#pragma once

#include <cstdint>

namespace devilution {

/**
 * @brief Estado del sistema atmosférico Nightmare
 */
struct NightmareAtmosphereState {
	bool enabled = true;                    ///< Sistema habilitado
	float desaturationLevel = 0.06f;        ///< Nivel de desaturación (6%)
	uint32_t windTick = 0;                  ///< Tick para animación de viento
	int shadowOffset = 0;                   ///< Offset actual de sombras por viento
	bool breathingActive = true;            ///< Respiración sutil de NPCs
};

// Estado global del sistema
extern NightmareAtmosphereState nightmareAtmosphere;

/**
 * @brief Inicializa el sistema atmosférico Nightmare
 */
void InitNightmareAtmosphere();

/**
 * @brief Actualiza los efectos atmosféricos
 * @param currentTick Tick actual del juego
 */
void UpdateNightmareAtmosphere(uint32_t currentTick);

/**
 * @brief Aplica desaturación Nightmare a un color
 * @param r Componente rojo (0-255)
 * @param g Componente verde (0-255)
 * @param b Componente azul (0-255)
 */
void ApplyNightmareDesaturation(uint8_t &r, uint8_t &g, uint8_t &b);

/**
 * @brief Obtiene el offset de sombra por viento
 * @return Offset en píxeles (-1, 0, +1)
 */
int GetWindShadowOffset();

/**
 * @brief Obtiene el offset de respiración para NPCs
 * @param npcId ID del NPC
 * @return Offset vertical en píxeles (±1)
 */
int GetNPCBreathingOffset(int npcId);

/**
 * @brief Activa/desactiva el sistema atmosférico
 * @param enabled true para activar
 */
void SetNightmareAtmosphereEnabled(bool enabled);

/**
 * @brief Verifica si el sistema está activo
 * @return true si está habilitado
 */
bool IsNightmareAtmosphereEnabled();

/**
 * @brief Configura el nivel de desaturación
 * @param level Nivel (0.0 = sin cambio, 0.1 = 10% desaturación)
 */
void SetDesaturationLevel(float level);

} // namespace devilution