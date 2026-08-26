#include "engine/render/weather_overlay.hpp"

#include <cstdlib>
#include <vector>

#include "engine/render/primitive_render.hpp"
#include "game_mode.hpp"
#include "options.h"

namespace devilution {
namespace {

struct RainDrop {
	float x;
	float y;
	float speedY;
	float speedX;
	int length;
	uint8_t color;
};

std::vector<RainDrop> rainPool;
bool rainInitialized = false;

void InitRainPool(int width, int height)
{
	constexpr size_t NumRainDrops = 350;
	rainPool.resize(NumRainDrops);

	for (auto &drop : rainPool) {
		drop.x = static_cast<float>(rand() % (width + 100) - 50);
		drop.y = static_cast<float>(rand() % height);
		drop.speedY = 12.0f + static_cast<float>(rand() % 8);
		drop.speedX = -2.5f - static_cast<float>(rand() % 3);
		drop.length = 5 + (rand() % 7);
		drop.color = (rand() % 2 == 0) ? 200 : 205; // Cold blue/silver palette index
	}
	rainInitialized = true;
}

} // namespace

void ResetWeatherRain()
{
	rainInitialized = false;
	rainPool.clear();
}

void RenderWeatherRain(const Surface &out)
{
	if (leveltype != DTYPE_TOWN)
		return;

	const int width = out.w();
	const int height = out.h();

	if (width <= 0 || height <= 0)
		return;

	if (!rainInitialized || rainPool.empty()) {
		InitRainPool(width, height);
	}

	for (auto &drop : rainPool) {
		drop.x += drop.speedX;
		drop.y += drop.speedY;

		if (drop.y >= height || drop.x < -50) {
			drop.y = static_cast<float>(-(rand() % 20));
			drop.x = static_cast<float>(rand() % (width + 100) - 20);
		}

		const int startX = static_cast<int>(drop.x);
		const int startY = static_cast<int>(drop.y);

		for (int i = 0; i < drop.length; i++) {
			const int px = startX + static_cast<int>(i * (drop.speedX / drop.length));
			const int py = startY + i;
			out.SetPixel({ px, py }, drop.color);
		}
	}
}

} // namespace devilution
