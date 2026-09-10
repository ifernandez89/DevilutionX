#include "floatingnumbers.h"

#include <cstdint>
#include <ctime>
#include <deque>
#include <fmt/format.h>
#include <string>

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "engine/render/text_render.hpp"
#include "options.h"
#include "utils/str_cat.hpp"

namespace devilution {

namespace {

struct FloatingNumber {
	Point startPos;
	Displacement startOffset;
	Displacement endOffset;
	std::string text;
	int accumulatedDamage;
	uint32_t time;
	uint32_t lastMerge;
	UiFlags style;
	int id;
	bool reverseDirection;
};

std::deque<FloatingNumber> FloatingQueue;

void ClearExpiredNumbers()
{
	const uint32_t now = SDL_GetTicks();
	std::erase_if(FloatingQueue, [now](const FloatingNumber &num) {
		return num.time <= now;
	});
}

GameFontTables GetGameFontSize(UiFlags flags)
{
	if (HasAnyOf(flags, UiFlags::FontSize30))
		return GameFont30;
	if (HasAnyOf(flags, UiFlags::FontSize24))
		return GameFont24;
	return GameFont12;
}

} // namespace

void AddFloatingNumber(Point pos, Displacement offset, std::string text, UiFlags style, int id, bool reverseDirection, int addDamage)
{
	ClearExpiredNumbers();

	Displacement endOffset;
	if (!reverseDirection)
		endOffset = { 0, -140 };
	else
		endOffset = { 0, 140 };

	const uint32_t now = SDL_GetTicks();
	for (auto &num : FloatingQueue) {
		if (id != 0 && num.id == id && (now - num.lastMerge <= 300)) {
			if (addDamage > 0 && num.accumulatedDamage > 0) {
				num.accumulatedDamage += addDamage;
				num.text = fmt::format("{:d}", num.accumulatedDamage);
			} else {
				num.text = text;
			}
			num.lastMerge = now;
			num.style = style | UiFlags::Outlined;
			num.startPos = pos;
			num.time = now + 2000;
			return;
		}
	}

	if (FloatingQueue.size() >= 50) {
		FloatingQueue.pop_front();
	}

	FloatingNumber num {
		pos, offset, endOffset, text, addDamage,
		static_cast<uint32_t>(now + 2500),
		now,
		style | UiFlags::Outlined, id, reverseDirection
	};
	FloatingQueue.push_back(num);
}

void DrawFloatingNumbers(const Surface &out, Point viewPosition, Displacement offset)
{
	const uint32_t now = SDL_GetTicks();
	for (auto &floatingNum : FloatingQueue) {
		if (floatingNum.time <= now)
			continue;

		Displacement worldOffset = viewPosition - floatingNum.startPos;
		worldOffset = worldOffset.worldToScreen() + offset + Displacement { TILE_WIDTH / 2, -TILE_HEIGHT / 2 } + floatingNum.startOffset;

		if (*GetOptions().Graphics.zoom) {
			worldOffset *= 2;
		}

		Point screenPosition { worldOffset.deltaX, worldOffset.deltaY };

		const int lineWidth = GetLineWidth(floatingNum.text, GetGameFontSize(floatingNum.style));
		screenPosition.x -= lineWidth / 2;
		const uint32_t timeLeft = floatingNum.time - now;
		const float mul = std::clamp(1.0f - (static_cast<float>(timeLeft) / 2500.0f), 0.0f, 1.0f);
		screenPosition += floatingNum.endOffset * mul;

		if (screenPosition.x + lineWidth <= 0 || screenPosition.x >= out.w() ||
		    screenPosition.y + 40 <= 0 || screenPosition.y >= out.h())
			continue;

		DrawString(out, floatingNum.text, Rectangle { screenPosition, { lineWidth, 0 } },
		    { .flags = floatingNum.style });
	}

	ClearExpiredNumbers();
}

void ClearFloatingNumbers()
{
	srand(static_cast<unsigned int>(time(nullptr)));

	FloatingQueue.clear();
}

} // namespace devilution
