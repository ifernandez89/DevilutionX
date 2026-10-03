#include "nightmare/world/cliff_skirt.hpp"
#include "engine/palette.h"
#include "lighting.h"
#include "levels/gendung.h"
#include <algorithm>
#include <cstdint>

namespace devilution {
namespace nightmare {

namespace {

// Fast deterministic integer hash for procedural rocky detail
inline uint32_t RockHash(int x, int y)
{
	uint32_t h = static_cast<uint32_t>(x * 374761393 + y * 668265263);
	h = (h ^ (h >> 13)) * 1274126177;
	return h ^ (h >> 16);
}

bool IsVoidOrOutOfBounds(Point p)
{
	if (!InDungeonBounds(p))
		return true;
	return dPiece[p.x][p.y] == 0;
}

void DrawRockColumn(const Surface &out, int screenX, int topY, int height, bool isSouthEastFace, int lightTableIndex, int worldSeedX, int worldSeedY)
{
	if (screenX < 0 || screenX >= out.w())
		return;

	const int clampedLight = std::clamp(lightTableIndex, 0, 15);
	const int endY = std::min(topY + height, out.h());

	for (int y = std::max(0, topY); y < endY; ++y) {
		int dy = y - topY;

		// Procedural rock texture calculation
		uint32_t h = RockHash(worldSeedX + screenX, worldSeedY + y);
		int grain = static_cast<int>(h & 7); // 0..7
		int strata = ((y + static_cast<int>((h >> 3) & 3)) / 6) & 3; // Sedimentary horizontal band

		// Base gray tone (PAL16_GRAY starts at 240, 240 is bright, 255 is dark)
		// Diablo grayscale: 240 (whitish gray) -> 248 (mid gray) -> 255 (dark gray/black)
		int baseShade = 244 + (grain >> 1) - (strata >> 1);

		// Cara differentiation (South-East has key highlight, South-West has ambient shadow)
		if (!isSouthEastFace) {
			baseShade += 2; // Darker face for SW
		}

		// Top edge rim highlight
		if (dy <= 1) {
			baseShade = isSouthEastFace ? 241 : 243;
		}

		// Vertical craggy fissures
		if (((h >> 6) % 17) == 0) {
			baseShade += 4; // Fissure crack
		}

		baseShade = std::clamp(baseShade, 240, 255);

		// Depth falloff: gradually degrade lighting into total darkness (pitch black abyss)
		int depthLightOffset = (dy * 16) / std::max(1, height);
		int effectiveLight = std::clamp(clampedLight + depthLightOffset, 0, 15);

		uint8_t finalColor = LightTables[effectiveLight][static_cast<uint8_t>(baseShade)];

		out.SetPixel({ screenX, y }, finalColor);
	}
}

} // namespace

void RenderCliffSkirts(const Surface &out, Point tilePosition, Point targetBufferPosition, int lightTableIndex)
{
	if (!InDungeonBounds(tilePosition) || dPiece[tilePosition.x][tilePosition.y] == 0)
		return;

	const bool swExposed = IsVoidOrOutOfBounds(tilePosition + Direction::SouthWest);
	const bool seExposed = IsVoidOrOutOfBounds(tilePosition + Direction::SouthEast);

	if (!swExposed && !seExposed)
		return;

	const int worldX = tilePosition.x * 64;
	const int worldY = tilePosition.y * 32;

	// South-West face: diagonal edge from (X, Y + 16) down to (X + 32, Y + 32)
	if (swExposed) {
		for (int u = 0; u < 32; ++u) {
			int px = targetBufferPosition.x + u;
			int py = targetBufferPosition.y + 16 + (u / 2) + 1; // +1 to start right below floor tile

			uint32_t colH = RockHash(worldX + u, worldY);
			int columnHeight = 88 + static_cast<int>(colH % 24); // 88..111 px drop

			DrawRockColumn(out, px, py, columnHeight, /*isSouthEastFace=*/false, lightTableIndex, worldX, worldY);
		}
	}

	// South-East face: diagonal edge from (X + 32, Y + 32) up to (X + 64, Y + 16)
	if (seExposed) {
		for (int u = 0; u < 32; ++u) {
			int px = targetBufferPosition.x + 32 + u;
			int py = targetBufferPosition.y + 32 - (u / 2); // Drops down from bottom to right rim

			uint32_t colH = RockHash(worldX + 32 + u, worldY + 100);
			int columnHeight = 88 + static_cast<int>(colH % 24); // 88..111 px drop

			DrawRockColumn(out, px, py, columnHeight, /*isSouthEastFace=*/true, lightTableIndex, worldX, worldY);
		}
	}
}

} // namespace nightmare
} // namespace devilution
