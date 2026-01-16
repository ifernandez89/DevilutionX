/**
 * @file ui_integration.h
 * @brief Nightmare Edition - UI Integration with Existing Systems
 * 
 * INTEGRACIÓN CON SISTEMAS EXISTENTES
 * ==================================
 * 
 * Sistema que integra los efectos contemplativos con los
 * menús y elementos UI existentes de DevilutionX.
 */

#pragma once

#include <cstdint>

namespace devilution {

/**
 * @brief Inicializa la integración con sistemas UI existentes
 */
void InitNightmareUIIntegration();

/**
 * @brief Maneja eventos de hover en menús principales
 * @param menuItem ID del elemento de menú
 */
void HandleMainMenuHover(int menuItem);

/**
 * @brief Maneja eventos de unhover en menús principales
 * @param menuItem ID del elemento de menú
 */
void HandleMainMenuUnhover(int menuItem);

/**
 * @brief Maneja clicks en menús principales con delay contemplativo
 * @param menuItem ID del elemento de menú
 * @return true si el click debe procesarse inmediatamente
 */
bool HandleMainMenuClick(int menuItem);

/**
 * @brief Maneja eventos de hover en menús de juego
 * @param menuItem ID del elemento de menú
 */
void HandleGameMenuHover(int menuItem);

/**
 * @brief Maneja eventos de unhover en menús de juego
 * @param menuItem ID del elemento de menú
 */
void HandleGameMenuUnhover(int menuItem);

/**
 * @brief Maneja clicks en menús de juego con delay contemplativo
 * @param menuItem ID del elemento de menú
 * @return true si el click debe procesarse inmediatamente
 */
bool HandleGameMenuClick(int menuItem);

/**
 * @brief Obtiene el offset de rendering para un elemento de menú
 * @param menuItem ID del elemento de menú
 * @return Offset en píxeles
 */
int GetMenuRenderOffset(int menuItem);

/**
 * @brief Obtiene el alpha de rendering para un elemento de menú
 * @param menuItem ID del elemento de menú
 * @return Valor alpha (0.0 - 1.0)
 */
float GetMenuRenderAlpha(int menuItem);

/**
 * @brief Obtiene la intensidad de glow para un elemento de menú
 * @param menuItem ID del elemento de menú
 * @return Intensidad de glow (0.0 - 1.0)
 */
float GetMenuGlowIntensity(int menuItem);

/**
 * @brief Verifica si un elemento de menú está en delay contemplativo
 * @param menuItem ID del elemento de menú
 * @return true si está en delay
 */
bool IsMenuInContemplativeDelay(int menuItem);

} // namespace devilution