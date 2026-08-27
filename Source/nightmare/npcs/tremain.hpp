#pragma once

#include "engine/point.hpp"
#include "player.h"

namespace devilution {
namespace nightmare {

enum class TremainState {
	Dormant,
	ActiveInTown,
	QuestGiven,
	FleshdoomKilled,
	CompletedAndDead
};

extern TremainState g_tremainState;
extern Point g_tremainPosition;

void InitTremainNPC();
void RenderTremainNPC(const Surface &out);
bool IsTmainInteractableAt(Point worldPos);
void InteractWithTremain(Player &player);
bool HasShadowfangItem(const Player &player);

} // namespace nightmare
} // namespace devilution
