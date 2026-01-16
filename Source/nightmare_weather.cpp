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
	LogVerbose("Nightmare Weather System - ALL EFFECTS DISABLED");
	
	nightmareWeather.enabled = false;  // 🚫 TODO DESHABILITADO
	nightmareWeather.lastUpdateTime = SDL_GetTicks();
	
	// Todos los sistemas deshabilitados
	nightmareWeather.rain.enabled = false;
	nightmareWeather.fog.enabled = false;
	nightmareWeather.particles.enabled = false;
	
	LogVerbose("🚫 ALL WEATHER EFFECTS DISABLED - Clean gameplay ready");
}

void UpdateNightmareWeather(float deltaTime)
{
	// 🚫 ALL EFFECTS DISABLED - No updates
	return;
}

void RenderNightmareWeather()
{
	// 🚫 ALL EFFECTS DISABLED - No rendering
	return;
}

// === SISTEMA DE LLUVIA ===

void InitRain()
{
	LogVerbose("🌧️ Rain system DISABLED by design choice");
	
	nightmareWeather.rain.enabled = false;  // 🚫 DESHABILITADA - No convence al usuario
	nightmareWeather.rain.intensity = 0.0f;
	nightmareWeather.rain.lastUpdateTime = SDL_GetTicks();
	
	LogVerbose("🌧️ Rain system disabled - focusing on other atmospheric effects");
}

void UpdateRain()
{
	// 🎯 USAR DIMENSIONES REALES DE PANTALLA
	const Surface &out = GlobalBackBuffer();
	const int gameViewportW = out.w();
	const int gameViewportH = out.h() - 144;  // Excluir panel inferior
	
	// Actualizar posición de cada gota - MOVIMIENTO MÁS REALISTA
	for (auto &drop : nightmareWeather.rain.drops) {
		// 🌧️ MOVIMIENTO VERTICAL con micro-variación horizontal
		drop.y += drop.speed;
		
		// 🌬️ MICRO-VIENTO: Variación horizontal muy sutil (±1 píxel ocasional)
		if (rand() % 20 == 0) {  // 5% de probabilidad por frame
			drop.x += (rand() % 3) - 1;  // -1, 0, o +1 píxel
		}
		
		// Reiniciar gota cuando sale de pantalla por abajo
		if (drop.y > gameViewportH) {
			drop.x = rand() % gameViewportW;
			drop.y = -(rand() % 100);  // Aparecer arriba con variación
			drop.speed = 1 + (rand() % 4);  // Nueva velocidad aleatoria
			drop.alpha = 40 + (rand() % 30); // Nuevo alpha aleatorio
		}
		
		// Mantener gota dentro de límites horizontales
		if (drop.x < 0) drop.x = 0;
		if (drop.x >= gameViewportW) drop.x = gameViewportW - 1;
	}
}

void DrawRain()
{
	// 🚫 LLUVIA DESHABILITADA - No renderizar nada
	return;
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
	
	nightmareWeather.fog.enabled = true;    // 🌫️ HABILITADA PERMANENTEMENTE
	nightmareWeather.fog.frame = 0;
	nightmareWeather.fog.lastFrameTime = SDL_GetTicks();
	nightmareWeather.fog.alpha = FOG_ALPHA_BASE;
	nightmareWeather.fog.intensity = 0.8f;  // Intensidad alta para visibilidad
	
	LogVerbose("🌫️ Fog initialized PERMANENTLY ACTIVE - {} frames, {}ms delay", FOG_FRAMES, FOG_FRAME_DELAY);
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
	
	if (currentTime - lastLogTime > 2000) {  // Log cada 2 segundos
		LogVerbose("🌫️ FOG ACTIVE: Drawing HIGHLY VISIBLE overlay - NIGHTMARE STYLE");
		lastLogTime = currentTime;
	}
	
	// 🌫️ IMPLEMENTACIÓN NIGHTMARE: Niebla MUY VISIBLE y atmosférica
	const Surface &out = GlobalBackBuffer();
	
	// 🎯 VIEWPORT CORRECTO: Solo renderizar en el área de juego
	const int gameViewportX = 0;
	const int gameViewportY = 0;
	const int gameViewportW = out.w();
	const int gameViewportH = out.h() - 144;  // Excluir panel inferior
	
	// Crear efecto de niebla con "sprites" simulados
	int fogPattern = nightmareWeather.fog.frame;
	
	// 🌫️ NIEBLA NIGHTMARE: Blobs GRANDES y MUY VISIBLES
	for (int y = gameViewportY; y < gameViewportH; y += 4) {  // Cada 4 píxeles (MUY DENSO)
		for (int x = gameViewportX; x < gameViewportW; x += 6) {  // Cada 6 píxeles
			// Patrón de "sprite" basado en posición y frame
			int spritePattern = (x + y + fogPattern * 4) % 13;
			
			// 🌫️ COBERTURA ALTA Y MUY VISIBLE
			if (spritePattern < 8) {  // ~60% de cobertura (MUY VISIBLE)
				// Verificar límites del área de juego
				if (x < gameViewportW && y < gameViewportH) {
					// 🌫️ COLORES GRISES MUY CONTRASTANTES Y VISIBLES
					uint8_t fogColor;
					if (spritePattern < 3) {
						fogColor = 160 + (spritePattern % 8);  // Grises oscuros 160-167
					} else {
						fogColor = 200 + (spritePattern % 10); // Grises claros 200-209
					}
					
					// 🌫️ "SPRITE" GRANDE: Blob de 4x4 píxeles para MÁXIMA visibilidad
					for (int dy = 0; dy < 4 && (y + dy) < gameViewportH; dy++) {
						for (int dx = 0; dx < 4 && (x + dx) < gameViewportW; dx++) {
							// Patrón de blob denso (casi todos los píxeles)
							if (spritePattern < 6 || (dx < 2 && dy < 2)) {
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

// === SISTEMA DE PARTÍCULAS FLOTANTES ===

void InitParticles()
{
	LogVerbose("✨ Initializing floating particles system");
	
	nightmareWeather.particles.enabled = true;
	nightmareWeather.particles.lastUpdateTime = SDL_GetTicks();
	nightmareWeather.particles.windDirection = 0.2f;  // Viento suave hacia la derecha
	nightmareWeather.particles.windStrength = 0.3f;   // Fuerza moderada
	
	// 🎯 USAR DIMENSIONES REALES DE PANTALLA
	const Surface &out = GlobalBackBuffer();
	const int gameViewportW = out.w();
	const int gameViewportH = out.h() - 144;  // Excluir panel inferior
	
	// Inicializar partículas flotantes con variación
	for (auto &particle : nightmareWeather.particles.particles) {
		particle.x = rand() % gameViewportW;
		particle.y = rand() % gameViewportH;
		particle.speedX = (rand() % 100 - 50) / 100.0f;  // -0.5 a 0.5
		particle.speedY = (rand() % 50 - 25) / 100.0f;   // -0.25 a 0.25
		particle.color = 140 + (rand() % 40);             // Grises 140-179
		particle.alpha = 30 + (rand() % 40);              // Alpha 30-69
		particle.life = 200 + (rand() % 300);             // Vida 200-499 frames
	}
	
	LogVerbose("✨ Particles initialized with {} floating elements", 
		nightmareWeather.particles.particles.size());
}

void UpdateParticles()
{
	// 🎯 USAR DIMENSIONES REALES DE PANTALLA
	const Surface &out = GlobalBackBuffer();
	const int gameViewportW = out.w();
	const int gameViewportH = out.h() - 144;  // Excluir panel inferior
	
	// Actualizar viento ocasionalmente
	static int windTimer = 0;
	windTimer++;
	if (windTimer > 300) {  // Cada 5 segundos aprox
		nightmareWeather.particles.windDirection += (rand() % 20 - 10) / 100.0f;
		nightmareWeather.particles.windStrength = 0.1f + (rand() % 40) / 100.0f;
		windTimer = 0;
	}
	
	// Actualizar cada partícula
	for (auto &particle : nightmareWeather.particles.particles) {
		// Aplicar viento
		particle.speedX += nightmareWeather.particles.windDirection * 0.01f;
		particle.speedY += (rand() % 3 - 1) / 100.0f;  // Movimiento vertical aleatorio
		
		// Actualizar posición
		particle.x += static_cast<int>(particle.speedX);
		particle.y += static_cast<int>(particle.speedY);
		
		// Reducir vida
		particle.life--;
		
		// Reiniciar partícula si sale de pantalla o muere
		if (particle.x < 0 || particle.x >= gameViewportW || 
		    particle.y < 0 || particle.y >= gameViewportH || 
		    particle.life <= 0) {
			
			// Reaparecer en posición aleatoria
			particle.x = rand() % gameViewportW;
			particle.y = rand() % gameViewportH;
			particle.speedX = (rand() % 100 - 50) / 100.0f;
			particle.speedY = (rand() % 50 - 25) / 100.0f;
			particle.color = 140 + (rand() % 40);
			particle.alpha = 30 + (rand() % 40);
			particle.life = 200 + (rand() % 300);
		}
	}
}

void DrawParticles()
{
	static uint32_t lastLogTime = 0;
	uint32_t currentTime = SDL_GetTicks();
	
	if (currentTime - lastLogTime > 4000) {  // Log cada 4 segundos
		LogVerbose("✨ PARTICLES ACTIVE: Drawing {} floating elements - NIGHTMARE ATMOSPHERE", 
			nightmareWeather.particles.particles.size());
		lastLogTime = currentTime;
	}
	
	// ✨ IMPLEMENTACIÓN NIGHTMARE: Partículas flotantes atmosféricas
	const Surface &out = GlobalBackBuffer();
	
	// 🎯 VIEWPORT CORRECTO: Solo renderizar en el área de juego
	const int gameViewportX = 0;
	const int gameViewportY = 0;
	const int gameViewportW = out.w();
	const int gameViewportH = out.h() - 144;  // Excluir panel inferior
	
	// Renderizar cada partícula
	for (const auto &particle : nightmareWeather.particles.particles) {
		// Verificar límites del viewport del juego
		if (particle.x >= gameViewportX && particle.x < gameViewportW && 
		    particle.y >= gameViewportY && particle.y < gameViewportH) {
			
			// ✨ PARTÍCULA COMO PEQUEÑO BLOB (2x2 píxeles)
			uint8_t particleColor = particle.color;
			
			// Renderizar blob de 2x2 con variación
			for (int dy = 0; dy < 2 && (particle.y + dy) < gameViewportH; dy++) {
				for (int dx = 0; dx < 2 && (particle.x + dx) < gameViewportW; dx++) {
					// Variación de color para efecto orgánico
					uint8_t finalColor = particleColor + (dx + dy);
					out.SetPixel({particle.x + dx, particle.y + dy}, finalColor);
				}
			}
		}
	}
}

void SetParticlesEnabled(bool enabled)
{
	nightmareWeather.particles.enabled = enabled;
	LogVerbose("✨ Particles: {}", enabled ? "enabled" : "disabled");
}

void SetNightmareWeatherEnabled(bool enabled)
{
	nightmareWeather.enabled = enabled;
	LogVerbose("🌦️ Nightmare Weather System: {}", enabled ? "enabled" : "disabled");
	
	if (!enabled) {
		// Deshabilitar todos los efectos
		nightmareWeather.rain.enabled = false;
		nightmareWeather.fog.enabled = false;
		nightmareWeather.particles.enabled = false;
	}
}

} // namespace devilution