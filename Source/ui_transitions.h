/**
 * @file ui_transitions.h
 * @brief Nightmare Edition - UI Transitions System
 * 
 * SISTEMA DE TRANSICIONES CONTEMPLATIVAS
 * ====================================
 * 
 * Sistema que implementa transiciones sutiles y contemplativas
 * para crear una experiencia de UI única en Nightmare Edition.
 * 
 * FILOSOFÍA NIGHTMARE:
 * - Transiciones lentas y deliberadas
 * - Micro-delays que crean anticipación
 * - Breathing effects en elementos seleccionados
 * - Subtle glow para elementos interactivos
 * - Todo debe sentirse contemplativo, no reactivo
 */

#pragma once

#include <cstdint>

namespace devilution {

/**
 * @brief Estados de transición para elementos UI
 */
enum class TransitionState {
	Idle,           ///< Estado normal, sin transición
	FadeIn,         ///< Apareciendo gradualmente
	FadeOut,        ///< Desapareciendo gradualmente
	Breathing,      ///< Efecto de respiración sutil
	Glowing,        ///< Efecto de brillo sutil
	Contemplating   ///< Estado contemplativo (micro-delay)
};

/**
 * @brief Configuración de una transición UI
 */
struct UITransition {
	TransitionState state = TransitionState::Idle;
	float progress = 0.0f;          ///< Progreso de la transición (0.0 - 1.0)
	float duration = 1.0f;          ///< Duración en segundos
	float intensity = 1.0f;         ///< Intensidad del efecto (0.0 - 1.0)
	uint32_t startTime = 0;         ///< Tiempo de inicio de la transición
	bool loop = false;              ///< Si la transición debe repetirse
};

/**
 * @brief Estado global del sistema de transiciones
 */
struct UITransitionsState {
	bool enabled = true;                    ///< Sistema habilitado
	float globalIntensity = 0.7f;          ///< Intensidad global (sutil)
	uint32_t lastUpdateTime = 0;           ///< Último tiempo de actualización
	
	// Transiciones específicas
	UITransition menuFade;                 ///< Fade de menús
	UITransition buttonHover;              ///< Hover de botones
	UITransition panelTransition;          ///< Transición de paneles
	UITransition contemplativeDelay;       ///< Delay contemplativo
};

// Estado global del sistema
extern UITransitionsState uiTransitions;

/**
 * @brief Inicializa el sistema de transiciones UI
 */
void InitUITransitions();

/**
 * @brief Actualiza todas las transiciones activas
 * @param deltaTime Tiempo transcurrido desde la última actualización
 */
void UpdateUITransitions(float deltaTime);

/**
 * @brief Actualiza una transición individual
 * @param transition Transición a actualizar
 * @param deltaTime Tiempo transcurrido
 */
void UpdateSingleTransition(UITransition &transition, float deltaTime);

/**
 * @brief Inicia una transición de fade contemplativo
 * @param transition Transición a configurar
 * @param fadeIn true para fade in, false para fade out
 * @param duration Duración en segundos (por defecto 1.5s)
 */
void StartContemplativeFade(UITransition &transition, bool fadeIn, float duration = 1.5f);

/**
 * @brief Inicia un efecto de breathing en un elemento
 * @param transition Transición a configurar
 * @param intensity Intensidad del breathing (0.0 - 1.0)
 */
void StartBreathingEffect(UITransition &transition, float intensity = 0.3f);

/**
 * @brief Inicia un efecto de glow sutil
 * @param transition Transición a configurar
 * @param intensity Intensidad del glow (0.0 - 1.0)
 */
void StartSubtleGlow(UITransition &transition, float intensity = 0.4f);

/**
 * @brief Inicia un delay contemplativo (micro-pausa antes de acción)
 * @param transition Transición a configurar
 * @param delayMs Delay en milisegundos (por defecto 150ms)
 */
void StartContemplativeDelay(UITransition &transition, uint32_t delayMs = 150);

/**
 * @brief Obtiene el valor alpha para un fade
 * @param transition Transición de fade
 * @return Valor alpha (0.0 - 1.0)
 */
float GetFadeAlpha(const UITransition &transition);

/**
 * @brief Obtiene el offset de breathing
 * @param transition Transición de breathing
 * @return Offset en píxeles (-1 a +1)
 */
int GetBreathingOffset(const UITransition &transition);

/**
 * @brief Obtiene la intensidad de glow
 * @param transition Transición de glow
 * @return Intensidad de glow (0.0 - 1.0)
 */
float GetGlowIntensity(const UITransition &transition);

/**
 * @brief Verifica si un delay contemplativo ha terminado
 * @param transition Transición de delay
 * @return true si el delay ha terminado
 */
bool IsContemplativeDelayComplete(const UITransition &transition);

/**
 * @brief Habilita/deshabilita el sistema de transiciones
 * @param enabled true para habilitar
 */
void SetUITransitionsEnabled(bool enabled);

/**
 * @brief Establece la intensidad global de todas las transiciones
 * @param intensity Intensidad global (0.0 - 1.0)
 */
void SetGlobalTransitionIntensity(float intensity);

} // namespace devilution