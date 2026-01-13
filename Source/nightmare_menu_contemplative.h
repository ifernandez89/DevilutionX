/**
 * @file nightmare_menu_contemplative.h
 * @brief Nightmare Edition - Contemplative Menu System
 * 
 * MENÚ COMO ANTESALA DEL INFIERNO
 * ===============================
 * 
 * El menú no es UI. Es una escena.
 * No "elegís opciones". Te preparás psicológicamente.
 * 
 * Filosofía:
 * - Respirar
 * - Incomodar  
 * - Ser lento
 * - Ser oscuro
 * - Nunca "brillar"
 */

#pragma once

#include <cstdint>

namespace devilution {

/**
 * @brief Estado del sistema de menú contemplativo
 */
struct ContemplativeMenuState {
	bool enabled;
	uint32_t startTime;
	uint32_t lastUpdateTime;
	
	// 🌫️ Fondo vivo
	struct {
		int fogFrame;
		uint32_t lastFogTime;
		int fogSeed;  // Semilla para variación sutil
		float pulseBrightness;
		uint32_t lastPulseTime;
	} background;
	
	// 📜 Texto que emerge
	struct {
		bool textVisible;
		uint32_t textStartTime;
		uint8_t textAlpha;
		int currentOption;
		uint32_t lastTextUpdate;
	} text;
	
	// 🌊 Efectos sutiles
	struct {
		int shimmerOffset;
		uint32_t lastShimmerTime;
		bool darkeningActive;
		float darkeningLevel;
	} effects;
};

// Estado global
extern ContemplativeMenuState contemplativeMenu;

// Funciones principales
void InitContemplativeMenu();
void UpdateContemplativeMenu(uint32_t currentTick);
void RenderContemplativeMenu();

// Capas de renderizado
enum class MenuVisualLayer {
	BACKGROUND,
	FOG,
	SHADOW,
	TEXT
};

void DrawMenuLayer(MenuVisualLayer layer);

// Funciones específicas
void DrawMenuBackground();
void DrawFogOverlay(int frame, uint8_t alpha);
void DrawMenuShadow();
void DrawEmergingText();

// Control de estado
void SetContemplativeMenuEnabled(bool enabled);
void TriggerMenuEntry();
void ResetMenuState();

// Configuración
namespace MenuConfig {
	constexpr uint32_t FOG_FRAME_DELAY = 800;     // 800ms por frame (muy lento)
	constexpr uint32_t PULSE_CYCLE_TIME = 4000;   // 4 segundos por ciclo de pulso
	constexpr uint32_t TEXT_DELAY = 1500;         // 1.5s antes de mostrar texto
	constexpr uint32_t TEXT_ALPHA_STEP = 32;      // Saltos de alpha discretos
	constexpr uint32_t SHIMMER_DELAY = 200;       // 200ms para shimmer
	constexpr float MAX_DARKENING = 0.15f;        // 15% oscurecimiento máximo
}

} // namespace devilution