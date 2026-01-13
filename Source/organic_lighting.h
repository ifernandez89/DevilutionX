/**
 * @file organic_lighting.h
 * @brief Nightmare Edition - Organic Lighting System
 * 
 * LUCES ORGÁNICAS CONTEMPLATIVAS
 * ==============================
 * 
 * Sistema de micro-parpadeo para hogueras y luces del town
 * que crea la sensación de vida sin afectar la jugabilidad.
 * 
 * PRINCIPIOS NIGHTMARE:
 * - Parece vivo, no afecta visión real
 * - Micro-parpadeo irregular
 * - Variación mínima en radio de luz
 * - Determinístico pero orgánico
 * 
 * FILOSOFÍA:
 * "El fuego respira, pero no distrae"
 */

#pragma once

#include <cstdint>

namespace devilution {

/**
 * @brief Estado del sistema de luces orgánicas
 */
struct OrganicLightingState {
	bool enabled = true;              ///< Sistema habilitado
	uint32_t lastFlickerTick = 0;     ///< Último tick de parpadeo
	int flickerValue = 0;             ///< Valor actual de parpadeo (-1, 0, +1)
	uint32_t nextFlickerDelay = 0;    ///< Delay hasta próximo parpadeo
};

// Estado global del sistema
extern OrganicLightingState organicLighting;

/**
 * @brief Inicializa el sistema de luces orgánicas
 */
void InitOrganicLighting();

/**
 * @brief Actualiza el sistema de parpadeo
 * @param currentTick Tick actual del juego
 */
void UpdateOrganicLighting(uint32_t currentTick);

/**
 * @brief Obtiene el modificador de parpadeo para aplicar al radio de luz
 * @return Modificador (-1, 0, +1)
 */
int GetLightFlickerModifier();

/**
 * @brief Verifica si una luz debe tener efecto orgánico
 * @param lightType Tipo de luz
 * @return true si debe parpadear
 */
bool ShouldLightFlicker(int lightType);

/**
 * @brief Activa/desactiva el sistema de luces orgánicas
 * @param enabled true para activar
 */
void SetOrganicLightingEnabled(bool enabled);

/**
 * @brief Verifica si el sistema está activo
 * @return true si está habilitado
 */
bool IsOrganicLightingEnabled();

} // namespace devilution