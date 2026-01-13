/**
 * @file ui_contemplative.h
 * @brief Nightmare Edition - Contemplative UI Effects
 * 
 * EFECTOS CONTEMPLATIVOS PARA UI
 * =============================
 * 
 * Sistema que aplica efectos contemplativos específicos
 * a elementos de la interfaz de usuario.
 * 
 * FILOSOFÍA NIGHTMARE:
 * - Micro-delays en respuestas de botones
 * - Breathing effects en elementos seleccionados
 * - Subtle glow en elementos interactivos
 * - Transiciones lentas y deliberadas
 */

#pragma once

#include <cstdint>
#include "ui_transitions.h"

namespace devilution {

/**
 * @brief Estado de un elemento UI contemplativo
 */
struct ContemplativeElement {
	UITransition transition;        ///< Transición activa
	bool isHovered = false;        ///< Si el elemento está bajo el cursor
	bool isSelected = false;       ///< Si el elemento está seleccionado
	bool isPressed = false;        ///< Si el elemento está siendo presionado
	uint32_t lastInteraction = 0;  ///< Último tiempo de interacción
};

/**
 * @brief Estado global del sistema contemplativo
 */
struct ContemplativeUIState {
	bool enabled = true;                    ///< Sistema habilitado
	float contemplationLevel = 0.7f;       ///< Nivel de contemplación (0.0 - 1.0)
	
	// Elementos específicos
	ContemplativeElement mainMenuButtons;   ///< Botones del menú principal
	ContemplativeElement gameMenuButtons;   ///< Botones del menú de juego
	ContemplativeElement inventoryPanels;   ///< Paneles de inventario
	ContemplativeElement dialogBoxes;       ///< Cajas de diálogo
};

// Estado global
extern ContemplativeUIState contemplativeUI;

/**
 * @brief Inicializa el sistema de UI contemplativa
 */
void InitContemplativeUI();

/**
 * @brief Actualiza todos los elementos contemplativos
 * @param deltaTime Tiempo transcurrido desde la última actualización
 */
void UpdateContemplativeUI(float deltaTime);

/**
 * @brief Maneja el hover sobre un elemento contemplativo
 * @param element Elemento que recibe hover
 */
void HandleContemplativeHover(ContemplativeElement &element);

/**
 * @brief Maneja cuando se deja de hacer hover sobre un elemento
 * @param element Elemento que pierde hover
 */
void HandleContemplativeUnhover(ContemplativeElement &element);

/**
 * @brief Maneja el click sobre un elemento contemplativo
 * @param element Elemento que recibe click
 * @return true si el click debe procesarse inmediatamente
 */
bool HandleContemplativeClick(ContemplativeElement &element);

/**
 * @brief Maneja la selección de un elemento contemplativo
 * @param element Elemento seleccionado
 */
void HandleContemplativeSelection(ContemplativeElement &element);

/**
 * @brief Obtiene el offset de rendering para un elemento contemplativo
 * @param element Elemento contemplativo
 * @return Offset en píxeles para aplicar al rendering
 */
int GetContemplativeRenderOffset(const ContemplativeElement &element);

/**
 * @brief Obtiene el alpha de rendering para un elemento contemplativo
 * @param element Elemento contemplativo
 * @return Valor alpha (0.0 - 1.0)
 */
float GetContemplativeAlpha(const ContemplativeElement &element);

/**
 * @brief Obtiene la intensidad de glow para un elemento contemplativo
 * @param element Elemento contemplativo
 * @return Intensidad de glow (0.0 - 1.0)
 */
float GetContemplativeGlow(const ContemplativeElement &element);

/**
 * @brief Verifica si un elemento está en delay contemplativo
 * @param element Elemento contemplativo
 * @return true si está en delay
 */
bool IsInContemplativeDelay(const ContemplativeElement &element);

/**
 * @brief Establece el nivel de contemplación global
 * @param level Nivel de contemplación (0.0 - 1.0)
 */
void SetContemplationLevel(float level);

/**
 * @brief Habilita/deshabilita el sistema contemplativo
 * @param enabled true para habilitar
 */
void SetContemplativeUIEnabled(bool enabled);

} // namespace devilution