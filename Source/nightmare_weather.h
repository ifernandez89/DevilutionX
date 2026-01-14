/**
 * @file nightmare_weather.h
 * @brief Nightmare Edition - Weather Effects (Rain & Fog)
 * 
 * EFECTOS CLIMÁTICOS NIGHTMARE
 * ===========================
 * 
 * Sistema de lluvia y niebla para crear atmósfera contemplativa
 * en Nightmare Edition.
 * 
 * FILOSOFÍA NIGHTMARE:
 * - Lluvia simple sin físicas complejas
 * - Niebla lenta e irregular
 * - Overlays sutiles y orgánicos
 * - Inquietante pero no distractivo
 */

#pragma once

#include <cstdint>
#include <array>

namespace devilution {

/**
 * @brief Estructura de una partícula flotante
 */
struct FloatingParticle {
	int x = 0;              ///< Posición X
	int y = 0;              ///< Posición Y
	float speedX = 0.0f;    ///< Velocidad horizontal
	float speedY = 0.0f;    ///< Velocidad vertical
	uint8_t color = 128;    ///< Color de la partícula
	uint8_t alpha = 50;     ///< Transparencia
	int life = 100;         ///< Vida restante
};

/**
 * @brief Estado del sistema de partículas flotantes
 */
struct ParticleState {
	bool enabled = true;                           ///< Sistema habilitado
	std::array<FloatingParticle, 32> particles;   ///< 32 partículas flotantes
	uint32_t lastUpdateTime = 0;                  ///< Último tiempo de actualización
	float windDirection = 0.0f;                   ///< Dirección del viento
	float windStrength = 0.5f;                    ///< Fuerza del viento
};

/**
 * @brief Estructura de una gota de lluvia (LEGACY - No usada)
 */
struct RainDrop {
	int x = 0;              ///< Posición X
	int y = 0;              ///< Posición Y
	int speed = 4;          ///< Velocidad de caída
	uint8_t alpha = 80;     ///< Transparencia
};

/**
 * @brief Estado del sistema de lluvia (DESHABILITADO)
 */
struct RainState {
	bool enabled = false;                       ///< Lluvia deshabilitada
	std::array<RainDrop, 64> drops;            ///< 64 gotas de lluvia (no usadas)
	uint32_t lastUpdateTime = 0;               ///< Último tiempo de actualización
	float intensity = 0.0f;                    ///< Intensidad de lluvia (0.0)
};

/**
 * @brief Estado del sistema de niebla
 */
struct FogState {
	bool enabled = false;                       ///< Niebla habilitada
	int frame = 0;                             ///< Frame actual de animación
	uint32_t lastFrameTime = 0;                ///< Último tiempo de cambio de frame
	uint8_t alpha = 32;                        ///< Transparencia de niebla
	float intensity = 0.7f;                    ///< Intensidad de niebla (0.0 - 1.0)
};

/**
 * @brief Estado global del sistema climático
 */
struct WeatherState {
	bool enabled = true;                        ///< Sistema climático habilitado
	RainState rain;                            ///< Estado de lluvia (deshabilitada)
	FogState fog;                              ///< Estado de niebla
	ParticleState particles;                   ///< Estado de partículas flotantes
	uint32_t lastUpdateTime = 0;               ///< Último tiempo de actualización
};

// Estado global
extern WeatherState nightmareWeather;

/**
 * @brief Inicializa el sistema climático Nightmare
 */
void InitNightmareWeather();

/**
 * @brief Actualiza todos los efectos climáticos
 * @param deltaTime Tiempo transcurrido desde la última actualización
 */
void UpdateNightmareWeather(float deltaTime);

/**
 * @brief Renderiza todos los efectos climáticos
 */
void RenderNightmareWeather();

// === SISTEMA DE PARTÍCULAS FLOTANTES ===

/**
 * @brief Inicializa el sistema de partículas flotantes
 */
void InitParticles();

/**
 * @brief Actualiza las partículas flotantes
 */
void UpdateParticles();

/**
 * @brief Renderiza las partículas flotantes
 */
void DrawParticles();

/**
 * @brief Habilita/deshabilita las partículas
 * @param enabled true para habilitar
 */
void SetParticlesEnabled(bool enabled);

// === SISTEMA DE LLUVIA (DESHABILITADO) ===

/**
 * @brief Inicializa el sistema de lluvia
 */
void InitRain();

/**
 * @brief Actualiza la lluvia
 */
void UpdateRain();

/**
 * @brief Renderiza la lluvia
 */
void DrawRain();

/**
 * @brief Habilita/deshabilita la lluvia
 * @param enabled true para habilitar
 */
void SetRainEnabled(bool enabled);

/**
 * @brief Establece la intensidad de lluvia
 * @param intensity Intensidad (0.0 - 1.0)
 */
void SetRainIntensity(float intensity);

// === SISTEMA DE NIEBLA ===

/**
 * @brief Inicializa el sistema de niebla
 */
void InitFog();

/**
 * @brief Actualiza la niebla
 * @param ticks Tiempo actual en ticks
 */
void UpdateFog(uint32_t ticks);

/**
 * @brief Renderiza la niebla
 */
void DrawFog();

/**
 * @brief Habilita/deshabilita la niebla
 * @param enabled true para habilitar
 */
void SetFogEnabled(bool enabled);

/**
 * @brief Establece la intensidad de niebla
 * @param intensity Intensidad (0.0 - 1.0)
 */
void SetFogIntensity(float intensity);

/**
 * @brief Habilita/deshabilita todo el sistema climático
 * @param enabled true para habilitar
 */
void SetNightmareWeatherEnabled(bool enabled);

} // namespace devilution