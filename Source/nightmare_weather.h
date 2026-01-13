#pragma once

#include <array>
#include <cstdint>

namespace devilution {

// Tipos de lluvia
enum class RainType : uint8_t {
	FINE = 0,    // Lluvia fina (40% de gotas)
	MEDIUM = 1,  // Lluvia media (40% de gotas)
	HEAVY = 2    // Lluvia pesada (20% de gotas)
};

// Capa de profundidad
enum class RainLayer : uint8_t {
	BACK = 0,    // Detrás de NPCs/jugador (60% de gotas)
	FRONT = 1    // Delante de NPCs/jugador (40% de gotas)
};

struct RainDrop {
	int x;              // Posición X
	int y;              // Posición Y
	int speed;          // Velocidad (1-5 píxeles/frame)
	int length;         // Longitud de la gota (1-6 píxeles)
	uint8_t alpha;      // Transparencia
	RainType type;      // Tipo de lluvia
	RainLayer layer;    // Capa de profundidad
	float windOffset;   // Offset horizontal por viento
};

struct GlobalWind {
	float direction;    // Dirección (-1.0 a 1.0, negativo = izquierda)
	float strength;     // Fuerza (0.0 a 1.0)
	uint32_t lastChange; // Último cambio de viento
};

struct WeatherState {
	bool enabled;
	uint32_t lastUpdateTime;

	struct {
		bool enabled;
		float intensity;
		uint32_t lastUpdateTime;
		std::array<RainDrop, 220> drops;  // 220 gotas
		GlobalWind wind;                   // Sistema de viento
	} rain;

	struct {
		bool enabled;
		int frame;
		uint32_t lastFrameTime;
		uint8_t alpha;
		float intensity;
	} fog;
};

extern WeatherState nightmareWeather;

// Funciones principales
void InitNightmareWeather();
void UpdateNightmareWeather(float deltaTime);
void RenderNightmareWeather();

// Funciones de control
void SetRainEnabled(bool enabled);
void SetFogEnabled(bool enabled);
void SetNightmareWeatherEnabled(bool enabled);
void SetRainIntensity(float intensity);
void SetFogIntensity(float intensity);

// Funciones de renderizado por capa
void DrawRainLayer(RainLayer layer);

} // namespace devilution
