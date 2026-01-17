#pragma once

#include <cstdint>
#include <string>

namespace devilution {

/**
 * @brief Corruption Detection System
 * 
 * Detecta si un save file está corrupto y fuerza reset a datos de fábrica
 * Filosofía: PREVENIR, no parchear
 */

/**
 * @brief Detecta si las coordenadas indican corrupción
 * @param x Coordenada X
 * @param y Coordenada Y
 * @return true si las coordenadas son claramente corruptas
 */
bool IsCorruptedCoordinate(int x, int y);

/**
 * @brief Detecta si el save file está corrupto
 * @param saveNumber Número del save file a verificar
 * @return true si el save está corrupto y debe resetearse
 */
bool IsSaveFileCorrupted(uint32_t saveNumber);

/**
 * @brief Fuerza reset completo a datos de fábrica
 * Resetea: niveles, quests, coordenadas del jugador
 */
void ForceFactoryReset();

/**
 * @brief Log de detección de corrupción
 * @param message Mensaje a loggear
 */
void LogCorruptionDetection(const std::string& message);

} // namespace devilution