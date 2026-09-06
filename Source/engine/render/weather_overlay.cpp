#include "engine/render/weather_overlay.hpp"

#include <cmath>
#include <cstdlib>
#include <vector>

#include "engine/render/primitive_render.hpp"
#include "game_mode.hpp"
#include "options.h"

namespace devilution {
namespace {

struct FogWisp {
	float x;
	float y;
	float speedX;
	float speedY;
	int width;
	int height;
	int density;
	uint8_t color;
};

std::vector<FogWisp> fogPool;
bool fogInitialized = false;

void InitFogPool(int screenWidth, int screenHeight)
{
	constexpr size_t NumFogWisps = 160;
	fogPool.resize(NumFogWisps);

	for (auto &wisp : fogPool) {
		wisp.x = static_cast<float>(rand() % (screenWidth + 200) - 100);
		wisp.y = static_cast<float>(rand() % screenHeight);
		wisp.speedX = 0.4f + static_cast<float>(rand() % 100) / 140.0f; // Soft gentle drift
		wisp.speedY = (static_cast<float>(rand() % 100) - 50.0f) / 500.0f; // Subtle vertical oscillation
		wisp.width = 18 + (rand() % 35);
		wisp.height = 3 + (rand() % 4);
		wisp.density = 2 + (rand() % 3);
		wisp.color = (rand() % 2 == 0) ? 200 : 205; // Cold translucent mist tint in standard palette
	}
	fogInitialized = true;
}

} // namespace

void ResetWeatherFog()
{
	fogInitialized = false;
	fogPool.clear();
}

void RenderWeatherFog(const Surface &out)
{
	if (leveltype != DTYPE_TOWN)
		return;

	const int screenWidth = out.w();
	const int screenHeight = out.h();

	if (screenWidth <= 0 || screenHeight <= 0)
		return;

	if (!fogInitialized || fogPool.empty()) {
		InitFogPool(screenWidth, screenHeight);
	}

	for (auto &wisp : fogPool) {
		wisp.x += wisp.speedX;
		wisp.y += wisp.speedY;

		if (wisp.x > screenWidth + 100) {
			wisp.x = static_cast<float>(-(rand() % 80 + 30));
			wisp.y = static_cast<float>(rand() % screenHeight);
		}
		if (wisp.y < 0)
			wisp.y = static_cast<float>(screenHeight - 1);
		else if (wisp.y >= screenHeight)
			wisp.y = 0;

		const int startX = static_cast<int>(wisp.x);
		const int startY = static_cast<int>(wisp.y);

		// Render soft dithered horizontal fog wisp
		for (int dy = 0; dy < wisp.height; dy++) {
			const int py = startY + dy;
			if (py < 0 || py >= screenHeight)
				continue;

			for (int dx = 0; dx < wisp.width; dx += wisp.density) {
				const int px = startX + dx + ((dy % 2) * (wisp.density / 2));
				if (px >= 0 && px < screenWidth) {
					out.SetPixel({ px, py }, wisp.color);
				}
			}
		}
	}
}

} // namespace devilution

