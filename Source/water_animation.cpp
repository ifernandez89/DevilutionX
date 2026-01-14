/**
 * @file water_animation.cpp
 * @brief Nightmare Edition - Animated Water Implementation
 * 
 * IMPLEMENTACIÓN DE AGUA ANIMADA CONTEMPLATIVA
 * ===========================================
 * 
 * Sistema que crea la ilusión de agua viva sin modificar
 * la lógica del juego, usando solo efectos visuales sutiles.
 */

#include "water_animation.h"

#include <cmath>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "utils/log.hpp"

namespace devilution {

// Estado global del sistema de agua animada
WaterAnimationState waterAnimation;

// Configuración del sistema
namespace {
	constexpr int WATER_FRAME_COUNT = 4;           // 4 frames de animación
	constexpr int WATER_FRAME_DELAY_MS = 200;      // 🔧 REFINADO: 200ms (era 50ms) - MÁS LENTO Y SUTIL
	constexpr float WAVE_FREQUENCY = 0.05f;        // 🔧 REFINADO: 0.05f (era 0.2f) - MÁS SUTIL
	constexpr int MAX_WAVE_OFFSET = 1;             // 🔧 REFINADO: ±1 píxel (era ±3) - SUTIL ORIGINAL
	
	// IDs de tiles de agua del town (estos valores son aproximados)
	// TODO: Verificar los IDs exactos en el código de DevilutionX
	constexpr int TOWN_WATER_TILE_START = 168;     // Primer tile de agua
	constexpr int TOWN_WATER_TILE_END = 175;       // Último tile de agua
}

void InitWaterAnimation()
{
	LogVerbose("Initializing Nightmare Water Animation System");
	
	waterAnimation.frame = 0;
	waterAnimation.lastUpdateTick = SDL_GetTicks();
	waterAnimation.enabled = true;
	waterAnimation.wavePhase = 0.0f;
	
	LogVerbose("Water Animation System initialized - Contemplative mode active");
}

void UpdateWaterAnimation(uint32_t currentTick)
{
	if (!waterAnimation.enabled) {
		return;
	}
	
	// Actualizar frame de animación (muy lento - 5 FPS)
	if (currentTick - waterAnimation.lastUpdateTick >= WATER_FRAME_DELAY_MS) {
		waterAnimation.frame = (waterAnimation.frame + 1) % WATER_FRAME_COUNT;
		waterAnimation.lastUpdateTick = currentTick;
		
		// Actualizar fase de onda para cálculos de oleaje
		waterAnimation.wavePhase += WAVE_FREQUENCY;
		if (waterAnimation.wavePhase > 6.28f) { // 2π
			waterAnimation.wavePhase -= 6.28f;
		}
	}
}

int GetWaterWaveOffset(int x, int y)
{
	if (!waterAnimation.enabled) {
		return 0;
	}
	
	// Cálculo de oleaje sutil usando función seno
	// Combina posición del tile con fase global para crear patrón orgánico
	float waveInput = (x + y + waterAnimation.wavePhase) * WAVE_FREQUENCY;
	float waveValue = std::sin(waveInput);
	
	// Convertir a offset de píxel - REFINADO PARA SUTILEZA
	if (waveValue > 0.5f) {
		return 1;  // 🔧 REFINADO: Solo ±1 píxel para máxima sutileza
	} else if (waveValue < -0.5f) {
		return -1; // 🔧 REFINADO: Solo ±1 píxel para máxima sutileza
	} else {
		return 0;
	}
}

bool IsTownWaterTile(int tileId)
{
	// 🚫 TEMPORALMENTE DESHABILITADO - Para eliminar bug del arroyo
	// MÉTODO SIMPLE: Rango conservador para agua del town
	// Aplicar efectos sutiles solo a tiles de agua conocidos
	
	// Rango principal de agua del town
	// if (tileId >= 168 && tileId <= 175) return true;
	
	// Rango secundario de fuentes y agua decorativa
	// if (tileId >= 200 && tileId <= 210) return true;
	
	return false;  // 🚫 DESHABILITADO: No aplicar efectos de agua por ahora
}

int GetWaterAnimationFrame()
{
	return waterAnimation.frame;
}

void SetWaterAnimationEnabled(bool enabled)
{
	LogVerbose("Water Animation: {}", enabled ? "enabled" : "disabled");
	waterAnimation.enabled = enabled;
	
	if (!enabled) {
		// Reset al estado inicial cuando se desactiva
		waterAnimation.frame = 0;
		waterAnimation.wavePhase = 0.0f;
	}
}

bool IsWaterAnimationEnabled()
{
	return waterAnimation.enabled;
}

} // namespace devilution