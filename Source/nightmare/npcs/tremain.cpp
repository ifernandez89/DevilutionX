#include "nightmare/npcs/tremain.hpp"

#include <cstring>

#include "diablo_msg.hpp"
#include "engine/clx_sprite.hpp"
#include "engine/load_clx.hpp"
#include "engine/sound.h"
#include "items.h"
#include "nightmare/restoration/registry.hpp"
#include "options.h"
#include "player.h"
#include "utils/language.h"

namespace devilution {
namespace nightmare {

TremainState g_tremainState = TremainState::ActiveInTown;
Point g_tremainPosition = { 62, 23 }; // Original Church Ruins position in Tristram

namespace {
OptionalOwnedClxSpriteList g_tremainSprites;
} // namespace

void InitTremainNPC()
{
	g_tremainState = TremainState::ActiveInTown;
	g_tremainPosition = { 62, 23 };

	if (!g_tremainSprites) {
		g_tremainSprites = LoadClx("towners\\priest8.clx");
		if (!g_tremainSprites) {
			g_tremainSprites = LoadClx("towners\\priest\\priest8.clx");
		}
	}
}

void RenderTremainNPC(const Surface &out)
{
	if (g_tremainState == TremainState::CompletedAndDead)
		return;

	// Tremain is present in Tristram
}

bool IsTmainInteractableAt(Point worldPos)
{
	if (g_tremainState == TremainState::CompletedAndDead)
		return false;

	return (worldPos.x >= g_tremainPosition.x - 1 && worldPos.x <= g_tremainPosition.x + 1 &&
	        worldPos.y >= g_tremainPosition.y - 1 && worldPos.y <= g_tremainPosition.y + 1);
}

bool HasShadowfangItem(const Player &player)
{
	for (int i = 0; i < player._pNumInv; i++) {
		if (strstr(player.InvList[i]._iIName, "Shadowfang") != nullptr || strstr(player.InvList[i]._iName, "Shadowfang") != nullptr)
			return true;
	}
	for (int i = 0; i < NUM_INVLOC; i++) {
		if (strstr(player.InvBody[i]._iIName, "Shadowfang") != nullptr || strstr(player.InvBody[i]._iName, "Shadowfang") != nullptr)
			return true;
	}
	return false;
}

void InteractWithTremain(Player &player)
{
	if (g_tremainState == TremainState::CompletedAndDead)
		return;

	if (HasShadowfangItem(player)) {
		// Resolution: priest07.wav
		PlayNightmareRestorationWav("sfx\\towners\\priest07.wav");
		g_tremainState = TremainState::CompletedAndDead;
		SpawnUnique(UITEM_BOVINE, player.position.tile);
		InitDiabloMsg(_("Tremain: Light be praised! Wait... It burns! Hellfire, consuming me!"));
	} else if (g_tremainState == TremainState::ActiveInTown) {
		// Initial Quest monologue: priest05.wav (76s)
		PlayNightmareRestorationWav("sfx\\towners\\priest05.wav");
		g_tremainState = TremainState::QuestGiven;
		InitDiabloMsg(_("Tremain: A demon named Fleshdoom wields Shadowfang... Do not succumb to its curse."));
	} else {
		// Urge: priest06.wav
		PlayNightmareRestorationWav("sfx\\towners\\priest06.wav");
		InitDiabloMsg(_("Tremain: Fleshdoom remains in the Catacombs. Destroy him, but do not wield Shadowfang."));
	}
}

} // namespace nightmare
} // namespace devilution
