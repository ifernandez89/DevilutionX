#include "nightmare_weather.h"

#include <cstdlib>
#include <algorithm>
#include <vector>

#include "engine/backbuffer_state.hpp"
#include "utils/log.hpp"
#include "levels/gendung.h"  // Para leveltype
#include "diablo.h"          // Para flags de pausa y menús

namespace devilution {

WeatherState nightmareWeather = {};

// Constantes
constexpr uint32_t FOG_FRAME_DELAY = 500;  // 500ms por frame
constexpr int FOG_FRAMES = 8;              // 8 frames de animación
constexpr uint8_t FOG_ALPHA_BASE = 32;     // Alpha base de niebla

void InitRain()
{
	const Surface &out = GlobalBackBuffer();
	const int gameViewportW = out.w();
	const int gameViewportH = out.h() - 144;

	// MEJORA 1: Densidad responsiva basada en resolución
	int targetDrops = (gameViewportW * gameViewportH) / 18000;
	targetDrops = std::clamp(targetDrops, 120, 300);
	nightmareWeather.rain.targetDropCount = targetDrops;
	
	// Redimensionar vector de gotas
	nightmareWeather.rain.drops.resize(targetDrops);

	for (size_t i = 0; i < nightmareWeather.rain.drops.size(); i++) {
		auto &drop = nightmareWeather.rain.drops[i];

		// Posición aleatoria
		drop.x = rand() % gameViewportW;
		drop.y = rand() % gameViewportH;

		// Distribución de tipos: 40% fina, 40% media, 20% pesada
		int typeRoll = rand() % 100;
		if (typeRoll < 40) {
			drop.type = RainType::FINE;
			drop.speed = 1 + (rand() % 2);  // 1-2 px/frame
			drop.length = 1 + (rand() % 2); // 1-2 píxeles
			drop.alpha = 60;
		} else if (typeRoll < 80) {
			drop.type = RainType::MEDIUM;
			drop.speed = 2 + (rand() % 2);  // 2-3 px/frame
			drop.length = 2 + (rand() % 3); // 2-4 píxeles
			drop.alpha = 80;
		} else {
			drop.type = RainType::HEAVY;
			drop.speed = 3 + (rand() % 3);  // 3-5 px/frame
			drop.length = 4 + (rand() % 3); // 4-6 píxeles
			drop.alpha = 100;
		}

		// Distribución de capas: 60% atrás, 40% adelante
		drop.layer = (rand() % 100 < 60) ? RainLayer::BACK : RainLayer::FRONT;

		// Inicializar offset de viento
		drop.windOffset = 0.0f;
	}

	// Inicializar viento global con sistema de interpolación
	nightmareWeather.rain.wind.direction = -0.15f; // Leve hacia la izquierda
	nightmareWeather.rain.wind.strength = 0.3f;    // Sutil
	nightmareWeather.rain.wind.lastChange = SDL_GetTicks();
	
	// MEJORA 2: Sistema de interpolación de viento
	nightmareWeather.rain.targetWindDirection = nightmareWeather.rain.wind.direction;
	nightmareWeather.rain.targetWindStrength = nightmareWeather.rain.wind.strength;
	nightmareWeather.rain.windTransition = 1.0f; // Completamente interpolado al inicio

	LogVerbose("🌧️ Rain initialized: {} drops (40% fine, 40% medium, 20% heavy) for {}x{}", 
		targetDrops, gameViewportW, gameViewportH);
}

void InitFog()
{
	nightmareWeather.fog.frame = 0;
	nightmareWeather.fog.alpha = FOG_ALPHA_BASE;
	nightmareWeather.fog.lastFrameTime = SDL_GetTicks();

	LogVerbose("🌫️ Fog initialized");
}

void InitNightmareWeather()
{
	nightmareWeather.enabled = true;
	nightmareWeather.context = WeatherContext::TOWN_IDLE;
	nightmareWeather.rain.enabled = true;
	nightmareWeather.rain.intensity = 0.7f;
	nightmareWeather.fog.enabled = false; // Desactivada por ahora
	nightmareWeather.fog.intensity = 0.8f;

	InitRain();
	InitFog();

	LogVerbose("🌙 Nightmare Weather System initialized with responsive density");
}

WeatherContext GetCurrentWeatherContext()
{
	// MEJORA 3: Regla de oro de Diablo - supresión silenciosa
	// El clima debe apagarse silenciosamente durante interacciones importantes
	
	// Verificar flags de pausa y menús (necesitamos incluir los headers apropiados)
	extern bool PauseMode;
	extern bool invflag;
	extern bool spselflag;
	extern bool qtextflag;
	extern bool stextflag;
	extern bool helpflag;
	extern bool msgflag;
	extern bool gmenu_is_active();
	
	if (PauseMode || invflag || spselflag || qtextflag || 
	    stextflag || helpflag || msgflag || gmenu_is_active()) {
		return WeatherContext::SUPPRESSED;
	}
	
	// Si estamos en Tristram sin interferencias, clima activo
	if (leveltype == DTYPE_TOWN) {
		return WeatherContext::TOWN_ACTIVE;
	}
	
	return WeatherContext::SUPPRESSED;
}

void UpdateWind(uint32_t currentTime)
{
	auto &wind = nightmareWeather.rain.wind;

	// Cambiar dirección del viento cada 8-12 segundos
	if (currentTime - wind.lastChange > 8000 + (rand() % 4000)) {
		// Nueva dirección sutil (-0.3 a 0.3)
		nightmareWeather.rain.targetWindDirection = -0.3f + (static_cast<float>(rand() % 100) / 100.0f * 0.6f);

		// Nueva fuerza sutil (0.2 a 0.5)
		nightmareWeather.rain.targetWindStrength = 0.2f + (static_cast<float>(rand() % 30) / 100.0f);

		wind.lastChange = currentTime;
		nightmareWeather.rain.windTransition = 0.0f; // Iniciar interpolación
	}

	// MEJORA 2: Interpolación suave del viento (1-2 segundos)
	if (nightmareWeather.rain.windTransition < 1.0f) {
		nightmareWeather.rain.windTransition += 1.0f / 120.0f; // ~2 segundos a 60 FPS
		if (nightmareWeather.rain.windTransition > 1.0f) {
			nightmareWeather.rain.windTransition = 1.0f;
		}
		
		// Interpolación suave (lerp)
		float t = nightmareWeather.rain.windTransition;
		wind.direction = wind.direction * (1.0f - t) + nightmareWeather.rain.targetWindDirection * t;
		wind.strength = wind.strength * (1.0f - t) + nightmareWeather.rain.targetWindStrength * t;
	}
}

void UpdateRain()
{
	const Surface &out = GlobalBackBuffer();
	const int gameViewportW = out.w();
	const int gameViewportH = out.h() - 144;

	uint32_t currentTime = SDL_GetTicks();
	UpdateWind(currentTime);

	const auto &wind = nightmareWeather.rain.wind;

	for (auto &drop : nightmareWeather.rain.drops) {
		// Movimiento vertical
		drop.y += drop.speed;

		// Movimiento horizontal por viento (muy sutil)
		drop.windOffset += wind.direction * wind.strength * 0.5f;

		// Limitar offset de viento a ±3 píxeles
		if (drop.windOffset > 3.0f) drop.windOffset = 3.0f;
		if (drop.windOffset < -3.0f) drop.windOffset = -3.0f;

		// Reciclaje cuando sale de pantalla
		if (drop.y > gameViewportH) {
			drop.x = rand() % gameViewportW;
			drop.y = -(rand() % 60);
			drop.windOffset = 0.0f; // Reset wind offset
		}
	}
}

void UpdateFog(uint32_t ticks)
{
	// Cambiar frame cada 500ms (muy lento para atmósfera)
	if (ticks - nightmareWeather.fog.lastFrameTime >= FOG_FRAME_DELAY) {
		nightmareWeather.fog.frame = (nightmareWeather.fog.frame + 1) % FOG_FRAMES;
		nightmareWeather.fog.lastFrameTime = ticks;
	}
}

void UpdateNightmareWeather(float deltaTime)
{
	// Solo activo en Tristram (DTYPE_TOWN)
	if (!nightmareWeather.enabled || leveltype != DTYPE_TOWN) return;

	// MEJORA 3: Actualizar contexto y manejar supresión inteligente
	WeatherContext newContext = GetCurrentWeatherContext();
	nightmareWeather.context = newContext;
	
	// Si estamos suprimidos, no actualizar gotas (dejar morir las existentes)
	if (newContext == WeatherContext::SUPPRESSED) {
		return; // Las gotas existentes seguirán cayendo pero no se actualizan ni respawnean
	}

	// Actualizar lluvia solo si no estamos suprimidos
	if (nightmareWeather.rain.enabled) {
		UpdateRain();
	}

	// Actualizar niebla
	if (nightmareWeather.fog.enabled) {
		UpdateFog(SDL_GetTicks());
	}
}

void DrawRainLayer(RainLayer layer)
{
	// Solo renderizar en Tristram (DTYPE_TOWN)
	if (leveltype != DTYPE_TOWN) return;
	
	// MEJORA 3: Respetar contexto de supresión también en capa frontal
	if (nightmareWeather.context == WeatherContext::SUPPRESSED) {
		return;
	}
	
	const Surface &out = GlobalBackBuffer();
	const int gameViewportW = out.w();
	const int gameViewportH = out.h() - 144;

	for (const auto &drop : nightmareWeather.rain.drops) {
		// Solo renderizar gotas de esta capa
		if (drop.layer != layer) continue;

		// Calcular posición X con offset de viento
		int finalX = drop.x + static_cast<int>(drop.windOffset);

		// Verificar límites
		if (finalX < 0 || finalX >= gameViewportW ||
		    drop.y < 0 || drop.y >= gameViewportH) {
			continue;
		}

		// Color según tipo - Optimizado para atmósfera de Tristram
		uint8_t waterColor;
		switch (drop.type) {
		case RainType::FINE:
			waterColor = 240 + (drop.y % 3);  // 240-242 (gris claro, sutil)
			break;
		case RainType::MEDIUM:
			waterColor = 242 + (drop.y % 3);  // 242-244 (gris medio)
			break;
		case RainType::HEAVY:
			waterColor = 244 + (drop.y % 4);  // 244-247 (gris más visible)
			break;
		}

		// Dibujar línea vertical
		for (int i = 0; i < drop.length && (drop.y + i) < gameViewportH; i++) {
			int drawX = finalX;
			int drawY = drop.y + i;

			if (drawX >= 0 && drawX < gameViewportW &&
			    drawY >= 0 && drawY < gameViewportH) {
				out.SetPixel({ drawX, drawY }, waterColor);
			}
		}
	}
}

void DrawFog()
{
	const Surface &out = GlobalBackBuffer();
	const int gameViewportW = out.w();
	const int gameViewportH = out.h() - 144;

	int fogPattern = nightmareWeather.fog.frame;

	// Sprites simulados cada 8x12 píxeles
	for (int y = 0; y < gameViewportH; y += 8) {
		for (int x = 0; x < gameViewportW; x += 12) {
			int spritePattern = (x + y + fogPattern * 2) % 23;

			// ~17% de cobertura
			if (spritePattern < 4) {
				if (x < gameViewportW && y < gameViewportH) {
					// Color gris claro visible
					uint8_t fogColor = 240 + (spritePattern % 8);

					// Blob de 2x2 píxeles
					for (int dy = 0; dy < 2 && (y + dy) < gameViewportH; dy++) {
						for (int dx = 0; dx < 2 && (x + dx) < gameViewportW; dx++) {
							if ((dx == 0 && dy == 0) || spritePattern < 2) {
								out.SetPixel({ x + dx, y + dy }, fogColor);
							}
						}
					}
				}
			}
		}
	}
}

void RenderNightmareWeather()
{
	// Solo renderizar en Tristram (DTYPE_TOWN)
	if (!nightmareWeather.enabled || leveltype != DTYPE_TOWN) return;

	// MEJORA 3: Respetar contexto de supresión
	// Si estamos suprimidos, no renderizar nuevas gotas
	if (nightmareWeather.context == WeatherContext::SUPPRESSED) {
		return;
	}

	// Renderizar niebla primero (fondo)
	if (nightmareWeather.fog.enabled) {
		DrawFog();
	}

	// Renderizar lluvia capa BACK (detrás de personajes)
	if (nightmareWeather.rain.enabled) {
		DrawRainLayer(RainLayer::BACK);
	}
}

// Funciones de control
void SetRainEnabled(bool enabled)
{
	nightmareWeather.rain.enabled = enabled;
	LogVerbose("🌧️ Rain {}", enabled ? "enabled" : "disabled");
}

void SetFogEnabled(bool enabled)
{
	nightmareWeather.fog.enabled = enabled;
	LogVerbose("🌫️ Fog {}", enabled ? "enabled" : "disabled");
}

void SetNightmareWeatherEnabled(bool enabled)
{
	nightmareWeather.enabled = enabled;
	LogVerbose("🌙 Nightmare Weather {}", enabled ? "enabled" : "disabled");
}

void SetRainIntensity(float intensity)
{
	nightmareWeather.rain.intensity = std::clamp(intensity, 0.0f, 1.0f);
	LogVerbose("🌧️ Rain intensity set to {:.2f}", nightmareWeather.rain.intensity);
}

void SetFogIntensity(float intensity)
{
	nightmareWeather.fog.intensity = std::clamp(intensity, 0.0f, 1.0f);
	LogVerbose("🌫️ Fog intensity set to {:.2f}", nightmareWeather.fog.intensity);
}

} // namespace devilution
