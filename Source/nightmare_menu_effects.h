/**
 * @file nightmare_menu_effects.h
 * @brief Nightmare Edition - Menu Visual Effects
 * 
 * EFECTOS VISUALES PARA MENÚ PRINCIPAL
 * ===================================
 * 
 * Sistema que aplica efectos contemplativos visibles
 * al menú principal de Nightmare Edition.
 * 
 * FILOSOFÍA NIGHTMARE:
 * - Breathing sutil en botones
 * - Fade contemplativo al entrar/salir
 * - Micro-delays en clicks
 * - Glow sutil en hover
 */

#pragma once

#include <cstdint>

namespace devilution {

/**
 * @brief Estado de efectos del menú Nightmare
 */
struct NightmareMenuState {
	bool enabled = true;                    ///< Efectos habilitados
	bool menuEntering = false;              ///< Menú apareciendo
	bool menuExiting = false;               ///< Menú desapareciendo
	float menuAlpha = 0.0f;                ///< Alpha del menú (0.0 - 1.0)
	int hoveredButton = -1;                ///< Botón bajo hover (-1 = ninguno)
	int selectedButton = -1;               ///< Botón seleccionado (-1 = ninguno)
	uint32_t lastUpdateTime = 0;           ///< Último tiempo de actualización
	uint32_t enterStartTime = 0;           ///< Tiempo de inicio de entrada
	
	// Estados de botones individuales
	float buttonAlpha[6] = {0.0f};         ///< Alpha de cada botón
	int buttonBreathingOffset[6] = {0};    ///< Offset de breathing por botón
	float buttonGlow[6] = {0.0f};          ///< Intensidad de glow por botón
	bool buttonDelayActive[6] = {false};   ///< Delay contemplativo activo
	uint32_t buttonClickTime[6] = {0};     ///< Tiempo de click por botón
};

// Estado global
extern NightmareMenuState nightmareMenu;

/**
 * @brief Inicializa los efectos del menú Nightmare
 */
void InitNightmareMenuEffects();

/**
 * @brief Actualiza todos los efectos del menú
 * @param deltaTime Tiempo transcurrido desde la última actualización
 */
void UpdateNightmareMenuEffects(float deltaTime);

/**
 * @brief Inicia la secuencia de entrada del menú
 */
void StartMenuEnterSequence();

/**
 * @brief Inicia la secuencia de salida del menú
 */
void StartMenuExitSequence();

/**
 * @brief Maneja hover sobre un botón del menú
 * @param buttonIndex Índice del botón (0-5)
 */
void HandleMenuButtonHover(int buttonIndex);

/**
 * @brief Maneja cuando se deja de hacer hover sobre un botón
 * @param buttonIndex Índice del botón (0-5)
 */
void HandleMenuButtonUnhover(int buttonIndex);

/**
 * @brief Maneja click en un botón del menú con delay contemplativo
 * @param buttonIndex Índice del botón (0-5)
 * @return true si el click debe procesarse inmediatamente
 */
bool HandleMenuButtonClick(int buttonIndex);

/**
 * @brief Obtiene el alpha de rendering para el menú completo
 * @return Valor alpha (0.0 - 1.0)
 */
float GetMenuAlpha();

/**
 * @brief Obtiene el alpha de rendering para un botón específico
 * @param buttonIndex Índice del botón (0-5)
 * @return Valor alpha (0.0 - 1.0)
 */
float GetMenuButtonAlpha(int buttonIndex);

/**
 * @brief Obtiene el offset de breathing para un botón específico
 * @param buttonIndex Índice del botón (0-5)
 * @return Offset en píxeles (-1 a +1)
 */
int GetMenuButtonBreathingOffset(int buttonIndex);

/**
 * @brief Obtiene la intensidad de glow para un botón específico
 * @param buttonIndex Índice del botón (0-5)
 * @return Intensidad de glow (0.0 - 1.0)
 */
float GetMenuButtonGlow(int buttonIndex);

/**
 * @brief Verifica si un botón está en delay contemplativo
 * @param buttonIndex Índice del botón (0-5)
 * @return true si está en delay
 */
bool IsMenuButtonInDelay(int buttonIndex);

/**
 * @brief Habilita/deshabilita los efectos del menú
 * @param enabled true para habilitar
 */
void SetNightmareMenuEffectsEnabled(bool enabled);

} // namespace devilution