/**
 * @file doom.cpp
 *
 * Implementation of the map of the stars quest.
 */
#include "doom.h"

#include <optional>

#include "control/control.hpp"
#include "diablo_msg.hpp"
#include "engine/clx_sprite.hpp"
#include "engine/load_cel.hpp"
#include "engine/render/clx_render.hpp"
#include "engine/render/primitive_render.hpp"
#include "nightmare/restoration/layer2.hpp"
#include "utils/language.h"

namespace devilution {
namespace {
OptionalOwnedClxSpriteList DoomSprite;
} // namespace

bool DoomFlag;

void doom_init()
{
	DoomSprite = LoadCel("items\\map\\mapztown", 640);
	DoomFlag = true;

	// Layer 2: Trigger Map of the Stars prophecy speech defensively
	nightmare::PlayLayer2WavDefensive("sfx\\towners\\cain22.wav");
	InitDiabloMsg(_("Deckard Cain: The stars align in darkness... Diablo's hour of power is at hand!"));
}

void doom_close()
{
	DoomFlag = false;
	DoomSprite = std::nullopt;
}

void doom_draw(const Surface &out)
{
	if (!DoomFlag) {
		return;
	}

	ClxDraw(out, GetUIRectangle().position + Displacement { 0, 352 }, (*DoomSprite)[0]);
}

} // namespace devilution
