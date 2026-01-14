/**
 * @file ui_nightmare.h
 * @brief Nightmare Edition UI Architecture - Layered Rendering System
 * 
 * NIGHTMARE UI ARCHITECTURE
 * ========================
 * 
 * Sistema de capas explícitas para DevilutionX que respeta la arquitectura original
 * pero permite mejoras visuales modernas de forma controlada.
 * 
 * FILOSOFÍA:
 * - Diablo-friendly: No rompe el código existente
 * - Capas explícitas: Orden claro de renderizado
 * - Sin widgets: Mantiene la simplicidad original
 * - Extensible: Fácil agregar efectos visuales
 * 
 * CAPAS NIGHTMARE:
 * - World: Mundo del juego + panel base
 * - Overlay: Tooltips, hints, debug info
 * - Screen: Inventory, spellbook, character
 * - Modal: Menús, diálogos
 * - Fade: Efectos de transición
 */

#pragma once

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

// Forward declarations for atmospheric systems
namespace devilution {
class WaterAnimationState;
class OrganicLightingState;
class NightmareAtmosphereState;

/**
 * @brief Capas de renderizado Nightmare Edition
 * 
 * Orden explícito de renderizado para UI, de fondo a primer plano.
 * Cada capa se renderiza completamente antes de la siguiente.
 */
enum class NightmareUILayer {
	World,    ///< Mundo del juego + panel base
	Overlay,  ///< Tooltips, hints, debug info
	Screen,   ///< Inventory, spellbook, character screen
	Modal,    ///< Menús, diálogos modales
	Fade      ///< Efectos de transición y fade
};

/**
 * @brief Estado global del sistema Nightmare UI
 */
struct NightmareUIState {
	bool enabled = false;           ///< Sistema Nightmare UI activado
	bool fadeActive = false;        ///< Fade effect activo
	float fadeAlpha = 0.0f;        ///< Alpha del fade (0.0 = transparente, 1.0 = opaco)
	bool animatedBgActive = false;  ///< Fondo animado activo
	int bgFrame = 0;               ///< Frame actual del fondo animado
	Uint32 lastFrameTime = 0;      ///< Tiempo del último frame para animación
};

// Estado global del sistema
extern NightmareUIState nightmareUI;

/**
 * @brief Inicializa el sistema Nightmare UI
 */
void InitNightmareUI();

/**
 * @brief Actualiza el estado del sistema Nightmare UI
 * @param deltaTime Tiempo transcurrido desde la última actualización
 */
void UpdateNightmareUI(float deltaTime);

/**
 * @brief Renderiza todas las capas UI en orden
 * 
 * Si Nightmare UI está desactivado, usa el renderizado original.
 * Si está activado, usa el sistema de capas con mejoras visuales.
 */
void RenderNightmareUI();

/**
 * @brief Renderiza una capa específica del sistema Nightmare
 * @param layer Capa a renderizar
 */
void RenderNightmareLayer(NightmareUILayer layer);

/**
 * @brief Activa/desactiva el sistema Nightmare UI
 * @param enabled True para activar, false para usar renderizado original
 */
void SetNightmareUIEnabled(bool enabled);

/**
 * @brief Inicia un efecto de fade
 * @param fadeIn True para fade in, false para fade out
 * @param duration Duración del fade en segundos
 */
void StartNightmareFade(bool fadeIn, float duration = 1.0f);

/**
 * @brief Activa/desactiva el fondo animado
 * @param active True para activar fondo animado
 */
void SetNightmareAnimatedBackground(bool active);

/**
 * @brief Activa el fade infernal para transiciones de menú
 */
void TriggerNightmareFadeTransition();

/**
 * @brief Verifica si hay una imagen personalizada disponible
 * @return true si existe custom_title.png
 */
bool HasCustomTitleImage();

/**
 * @brief Obtiene el frame actual de la animación de fondo
 * @return Frame actual (0-15)
 */
int GetNightmareBackgroundFrame();

/**
 * @brief Obtiene el alpha actual del fade effect
 * @return Alpha value (0.0-1.0)
 */
float GetNightmareFadeAlpha();

/**
 * @brief Inicializa todos los sistemas atmosféricos Nightmare
 */
void InitNightmareAtmosphericSystems();

/**
 * @brief Actualiza todos los sistemas atmosféricos
 * @param currentTick Tick actual del juego
 */
void UpdateNightmareAtmosphericSystems(uint32_t currentTick);

/**
 * @brief Activa/desactiva todos los efectos atmosféricos
 * @param enabled true para activar
 */
void SetNightmareAtmosphericEffectsEnabled(bool enabled);

} // namespace devilution