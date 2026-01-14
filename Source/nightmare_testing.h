/**
 * @file nightmare_testing.h
 * @brief Nightmare Edition - Testing & Demo System
 * 
 * SISTEMA DE TESTING Y DEMOSTRACIÓN
 * ================================
 * 
 * Sistema para probar y demostrar todos los efectos
 * Nightmare de manera visible e inmediata.
 */

#pragma once

#include <cstdint>

namespace devilution {

/**
 * @brief Inicializa el sistema de testing Nightmare
 */
void InitNightmareTesting();

/**
 * @brief Actualiza el sistema de testing
 * @param deltaTime Tiempo transcurrido
 */
void UpdateNightmareTesting(float deltaTime);

/**
 * @brief Inicia una demostración de efectos de menú
 */
void StartMenuEffectsDemo();

/**
 * @brief Inicia una demostración de efectos climáticos
 */
void StartWeatherEffectsDemo();

/**
 * @brief Simula hover en botón de menú para testing
 * @param buttonIndex Índice del botón (0-5)
 */
void TestMenuButtonHover(int buttonIndex);

/**
 * @brief Simula click en botón de menú para testing
 * @param buttonIndex Índice del botón (0-5)
 */
void TestMenuButtonClick(int buttonIndex);

/**
 * @brief Activa/desactiva lluvia para testing
 */
void ToggleRainForTesting();

/**
 * @brief Activa/desactiva niebla para testing
 */
void ToggleFogForTesting();

/**
 * @brief Muestra el estado de todos los sistemas Nightmare
 */
void ShowNightmareSystemsStatus();

} // namespace devilution