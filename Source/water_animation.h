/**
 * @file water_animation.h
 * @brief Nightmare Edition - Animated Water System
 * 
 * AGUA ANIMADA CONTEMPLATIVA
 * =========================
 * 
 * Sistema de animación sutil del agua en Tristram que respeta
 * completamente la filosofía original de Diablo.
 * 
 * PRINCIPIOS NIGHTMARE:
 * - El mundo no cambia, el jugador percibe que cambia
 * - Oleaje leve (±1 píxel máximo)
 * - Oscilación muy lenta, no flujo
 * - Determinístico y global
 * - Sin afectar gameplay, pathfinding o colisiones
 * 
 * FILOSOFÍA:
 * "El jugador lo siente, no lo ve"
 */

#pragma once

#include <cstdint>

namespace devilution {

/**
 * @brief Estado global de la animación de agua
 */
struct WaterAnimationState {
	int frame = 0;                    ///< Frame actual de animación (0-3)
	uint32_t lastUpdateTick = 0;      ///< Último tick de actualización
	bool enabled = true;              ///< Sistema habilitado
	float wavePhase = 0.0f;           ///< Fase de la onda para cálculos
};

// Estado global del sistema
extern WaterAnimationState waterAnimation;

/**
 * @brief Inicializa el sistema de agua animada
 */
void InitWaterAnimation();

/**
 * @brief Actualiza la animación de agua
 * @param currentTick Tick actual del juego
 */
void UpdateWaterAnimation(uint32_t currentTick);

/**
 * @brief Calcula el offset de oleaje para una posición
 * @param x Coordenada X del tile
 * @param y Coordenada Y del tile
 * @return Offset en píxeles (-1, 0, o +1)
 */
int GetWaterWaveOffset(int x, int y);

/**
 * @brief Verifica si un tile es agua del town
 * @param tileId ID del tile a verificar
 * @return true si es tile de agua
 */
bool IsTownWaterTile(int tileId);

/**
 * @brief Obtiene el frame de animación actual
 * @return Frame actual (0-3)
 */
int GetWaterAnimationFrame();

/**
 * @brief Activa/desactiva la animación de agua
 * @param enabled true para activar
 */
void SetWaterAnimationEnabled(bool enabled);

/**
 * @brief Verifica si la animación está activa
 * @return true si está habilitada
 */
bool IsWaterAnimationEnabled();

} // namespace devilution