/**
 * @file nightmare_weather.cpp
 * @brief Nightmare Edition - Weather Effects Implementation
 * 
 * IMPLEMENTACIÓN DE EFECTOS CLIMÁTICOS
 * ===================================
 * 
 * Sistema de lluvia y niebla para atmósfera contemplativa.
 */

#include "nightmare_weather.h"

#include <cstdlib>
#include <algorithm>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "utils/log.hpp"
#include "engine/dx.h"  // 🌧️ Para GlobalBackBuffer()

namespace devilution {

// Estado global del sistema climático
WeatherState nightmareWeather;

// Configuración del sistema
namespace {
	constexpr int SCREEN_WIDTH = 800;          // Ancho de pantalla estimado
	constexpr int SCREEN_HEIGHT = 600;         // Alto de pantalla estimado
	constexpr int RAIN_SPEED_BASE = 2;         // Velocidad MUY lenta (Diablo-style)
	constexpr int RAIN_SPEED_VARIANCE = 2;     // Variación mínima (2-4 píxeles/frame)
	constexpr uint32_t FOG_FRAME_DELAY = 500;  // 500ms por frame (MUY lento)
	constexpr int FOG_FRAMES = 8;              // 8 frames de animación de niebla
	constexpr uint8_t RAIN_ALPHA = 80;         // Alpha de gotas de lluvia
	constexpr uint8_t FOG_ALPHA_BASE = 32;     // Alpha base de niebla
}

void InitNightmareWeather()
{
	LogVerbose("Initializing Nightmare Weather System");
	
	nightmareWeather.enabled = true;
	nightmareWeather.lastUpdateTime = SDL_GetTicks();
	
	// Inicializar lluvia y niebla
	InitRain();
	InitFog();
	
	// 🌧️ ACTIVAR LLUVIA PERMANENTEMENTE para testing
	nightmareWeather.rain.enabled = true;
	nightmareWeather.rain.intensity = 0.7f;  // Intensidad alta para visibilidad
	
	// 🌫️ ACTIVAR NIEBLA PERMANENTEMENTE para testing
	nightmareWeather.fog.enabled = true;
	nightmareWeather.fog.intensity = 0.8f;   // Intensidad alta para visibilidad
	
	LogVerbose("🌧️ Nightmare Weather System initialized");
	LogVerbose("🌫️ Rain and fog effects PERMANENTLY ACTIVE for testing");
}

void UpdateNightmareWeather(float deltaTime)
{
	if (!nightmareWeather.enabled) {
		return;
	}
	
	uint32_t currentTime = SDL_GetTicks();
	nightmareWeather.lastUpdateTime = currentTime;
	
	// Actualizar lluvia
	if (nightmareWeather.rain.enabled) {
		UpdateRain();
	}
	
	// Actualizar niebla
	if (nightmareWeather.fog.enabled) {
		UpdateFog(currentTime);
	}
}

void RenderNightmareWeather()
{
	if (!nightmareWeather.enabled) {
		return;
	}
	
	// Renderizar niebla primero (fondo)
	if (nightmareWeather.fog.enabled) {
		DrawFog();
	}
	
	// Renderizar lluvia encima
	if (nightmareWeather.rain.enabled) {
		DrawRain();
	}
}

// === SISTEMA DE LLUVIA ===

void InitRain()
{
	LogVerbose("🌧️ Initializing rain system");
	
	nightmareWeather.rain.enabled = false;  // Deshabilitada por defecto
	nightmareWeather.rain.intensity = 0.5f;
	nightmareWeather.rain.lastUpdateTime = SDL_GetTicks();
	
	// 🎯 USAR DIMENSIONES REALES DE PANTALLA
	const Surface &out = GlobalBackBuffer();
	const int gameViewportW = out.w();
	const int gameViewportH = out.h() - 144;  // Excluir panel inferior
	
	// Inicializar gotas de lluvia en posiciones aleatorias
	for (auto &drop : nightmareWeather.rain.drops) {
		drop.x = rand() % gameViewportW;
		drop.y = rand() % gameViewportH;
		drop.speed = RAIN_SPEED_BASE + (rand() % RAIN_SPEED_VARIANCE);
		drop.alpha = RAIN_ALPHA;
	}
	
	LogVerbose("🌧️ Rain initialized with {} drops (viewport: {}x{})", 
		nightmareWeather.rain.drops.size(), gameViewportW, gameViewportH);
}

void UpdateRain()
{
	// 🎯 USAR DIMENSIONES REALES DE PANTALLA
	const Surface &out = GlobalBackBuffer();
	const int gameViewportW = out.w();
	const int gameViewportH = out.h() - 144;  // Excluir panel inferior
	
	// Actualizar posición de cada gota - MOVIMIENTO VERTICAL hacia abajo
	for (auto &drop : nightmareWeather.rain.drops) {
		// 🌧️ MOVIMIENTO VERTICAL: Solo actualizar Y (hacia abajo)
		drop.y += drop.speed;
		
		// Reiniciar gota cuando sale de pantalla por abajo
		if (drop.y > gameViewportH) {
			drop.x = rand() % gameViewportW;
			drop.y = -(rand() % 60);  // Aparecer más arriba de la pantalla
			drop.speed = RAIN_SPEED_BASE + (rand() % RAIN_SPEED_VARIANCE);
		}
	}
}

void DrawRain()
{
	static uint32_t lastLogTime = 0;
	uint32_t currentTime = SDL_GetTicks();
	
	if (currentTime - lastLogTime > 2000) {  // Log cada 2 segundos
		LogVerbose("🌧️ RAIN ACTIVE: Drawing {} drops - DIABLO STYLE", 
			nightmareWeather.rain.drops.size());
		lastLogTime = currentTime;
	}
	
	// 🌧️ IMPLEMENTACIÓN DIABLO-STYLE: Pool fijo, reciclaje, líneas simples
	const Surface &out = GlobalBackBuffer();
	
	// 🎯 VIEWPORT CORRECTO: Solo renderizar en el área de juego (no en UI)
	const int gameViewportX = 0;
	const int gameViewportY = 0;
	const int gameViewportW = out.w();
	const int gameViewportH = out.h() - 144;  // Excluir panel inferior (144px)
	
	// Renderizar cada gota como línea vertical AZUL (agua real)
	for (const auto &drop : nightmareWeather.rain.drops) {
		// Verificar límites del viewport del juego
		if (drop.x >= gameViewportX && drop.x < gameViewportW && 
		    drop.y >= gameViewportY && drop.y < gameViewportH) {
			
			// 🌧️ COLOR AZUL AGUA REAL (azules/celestes)
			uint8_t waterColor = 200 + (drop.y % 8);  // Azules 200-207
			
			// 🌧️ LÍNEA VERTICAL PARA SIMULAR CAÍDA
			int lineLength = 2 + (drop.speed / 3);  // 2-5 píxeles
			lineLength = std::min(lineLength, 5);
			
			for (int i = 0; i < lineLength && (drop.y + i) < gameViewportH; i++) {
				out.SetPixel({drop.x, drop.y + i}, waterColor);
			}
		}
	}
}

void SetRainEnabled(bool enabled)
{
	nightmareWeather.rain.enabled = enabled;
	LogVerbose("🌧️ Rain: {}", enabled ? "enabled" : "disabled");
}

void SetRainIntensity(float intensity)
{
	nightmareWeather.rain.intensity = std::clamp(intensity, 0.0f, 1.0f);
	
	// Ajustar alpha de gotas basado en intensidad
	uint8_t newAlpha = static_cast<uint8_t>(RAIN_ALPHA * nightmareWeather.rain.intensity);
	for (auto &drop : nightmareWeather.rain.drops) {
		drop.alpha = newAlpha;
	}
	
	LogVerbose("🌧️ Rain intensity set to: {:.2f}", nightmareWeather.rain.intensity);
}

// === SISTEMA DE NIEBLA ===

void InitFog()
{
	LogVerbose("🌫️ Initializing fog system");
	
	nightmareWeather.fog.enabled = true;   // Habilitada por defecto
	nightmareWeather.fog.frame = 0;
	nightmareWeather.fog.lastFrameTime = SDL_GetTicks();
	nightmareWeather.fog.alpha = FOG_ALPHA_BASE;
	nightmareWeather.fog.intensity = 0.7f;
	
	LogVerbose("🌫️ Fog initialized - {} frames, {}ms delay", FOG_FRAMES, FOG_FRAME_DELAY);
}

void UpdateFog(uint32_t ticks)
{
	// Actualizar frame de niebla cada 300ms (muy lento)
	if (ticks - nightmareWeather.fog.lastFrameTime >= FOG_FRAME_DELAY) {
		nightmareWeather.fog.frame = (nightmareWeather.fog.frame + 1) % FOG_FRAMES;
		nightmareWeather.fog.lastFrameTime = ticks;
		
		LogVerbose("🌫️ Fog frame updated: {}/{}", nightmareWeather.fog.frame, FOG_FRAMES);
	}
}

void DrawFog()
{
	static uint32_t lastLogTime = 0;
	uint32_t currentTime = SDL_GetTicks();
	
	if (currentTime - lastLogTime > 3000) {  // Log cada 3 segundos
		LogVerbose("🌫️ FOG ACTIVE: Drawing overlay - DIABLO STYLE");
		lastLogTime = currentTime;
	}
	
	// 🌫️ IMPLEMENTACIÓN DIABLO-STYLE: Sprites simulados, movimiento lento
	const Surface &out = GlobalBackBuffer();
	
	// 🎯 VIEWPORT CORRECTO: Solo renderizar en el área de juego
	const int gameViewportX = 0;
	const int gameViewportY = 0;
	const int gameViewportW = out.w();
	const int gameViewportH = out.h() - 144;  // Excluir panel inferior
	
	// Crear efecto de niebla con "sprites" simulados
	int fogPattern = nightmareWeather.fog.frame;
	
	// 🌫️ NIEBLA DIABLO-STYLE: Blobs grises claros que se mueven lentamente
	for (int y = gameViewportY; y < gameViewportH; y += 8) {  // Cada 8 píxeles
		for (int x = gameViewportX; x < gameViewportW; x += 12) {  // Cada 12 píxeles
			// Patrón de "sprite" basado en posición y frame
			int spritePattern = (x + y + fogPattern * 2) % 23;
			
			// 🌫️ COBERTURA SUTIL PERO VISIBLE (Diablo-style)
			if (spritePattern < 4) {  // ~17% de cobertura
				// Verificar límites del área de juego
				if (x < gameViewportW && y < gameViewportH) {
					// 🌫️ COLOR GRIS CLARO VISIBLE (más claro que antes)
					uint8_t fogColor = 240 + (spritePattern % 8);  // Grises claros 240-247
					
					// 🌫️ "SPRITE" SIMULADO: Blob de 2x2 píxeles
					for (int dy = 0; dy < 2 && (y + dy) < gameViewportH; dy++) {
						for (int dx = 0; dx < 2 && (x + dx) < gameViewportW; dx++) {
							// Patrón de blob (centro más denso)
							if ((dx == 0 && dy == 0) || spritePattern < 2) {
								out.SetPixel({x + dx, y + dy}, fogColor);
							}
						}
					}
				}
			}
		}
	}
}

void SetFogEnabled(bool enabled)
{
	nightmareWeather.fog.enabled = enabled;
	LogVerbose("🌫️ Fog: {}", enabled ? "enabled" : "disabled");
}

void SetFogIntensity(float intensity)
{
	nightmareWeather.fog.intensity = std::clamp(intensity, 0.0f, 1.0f);
	
	// Ajustar alpha basado en intensidad
	nightmareWeather.fog.alpha = static_cast<uint8_t>(FOG_ALPHA_BASE * nightmareWeather.fog.intensity);
	
	LogVerbose("🌫️ Fog intensity set to: {:.2f} (alpha: {})", 
		nightmareWeather.fog.intensity, nightmareWeather.fog.alpha);
}

void SetNightmareWeatherEnabled(bool enabled)
{
	nightmareWeather.enabled = enabled;
	LogVerbose("🌦️ Nightmare Weather System: {}", enabled ? "enabled" : "disabled");
	
	if (!enabled) {
		// Deshabilitar todos los efectos
		nightmareWeather.rain.enabled = false;
		nightmareWeather.fog.enabled = false;
	}
}

} // namespace devilution