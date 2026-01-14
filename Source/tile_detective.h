/**
 * @file tile_detective.h
 * @brief Nightmare Edition - Tile Detective System
 * 
 * SISTEMA DE DETECCIÓN INTELIGENTE DE TILES
 * ========================================
 * 
 * Sistema temporal para identificar programáticamente los tile IDs
 * exactos del arroyo de Tristram sin usar coordenadas hardcodeadas.
 * 
 * FILOSOFÍA NIGHTMARE:
 * - Detección basada en datos reales del juego
 * - Sin modificar el mundo original
 * - Instrumentación elegante y removible
 */

#pragma once

#include <cstdint>
#include <unordered_set>

namespace devilution {

/**
 * @brief Sistema de detección de tiles del arroyo
 */
struct TileDetectiveState {
	bool enabled = false;                           ///< Sistema de logging activo
	bool captureMode = false;                       ///< Modo captura activo
	std::unordered_set<uint16_t> detectedTiles;     ///< Tiles detectados
	int captureRadius = 2;                          ///< Radio de captura alrededor del jugador
};

// Estado global del detective
extern TileDetectiveState tileDetective;

/**
 * @brief Inicializa el sistema detective
 */
void InitTileDetective();

/**
 * @brief Activa/desactiva el modo captura
 * @param enabled true para activar captura
 */
void SetTileDetectiveCapture(bool enabled);

/**
 * @brief Registra un tile en la posición del jugador
 * @param tileId ID del tile
 * @param x Coordenada X
 * @param y Coordenada Y
 */
void LogTileAtPosition(uint16_t tileId, int x, int y);

/**
 * @brief Verifica si un tile fue detectado como agua
 * @param tileId ID del tile a verificar
 * @return true si fue detectado como agua del arroyo
 */
bool IsDetectedWaterTile(uint16_t tileId);

/**
 * @brief Exporta la lista de tiles detectados
 */
void ExportDetectedTiles();

/**
 * @brief Limpia la lista de tiles detectados
 */
void ClearDetectedTiles();

/**
 * @brief Actualiza el sistema detective (llamar desde GameLogic)
 */
void UpdateTileDetective();

} // namespace devilution