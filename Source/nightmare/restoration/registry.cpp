#include "nightmare/restoration/registry.hpp"

#include <memory>

#include "engine/random.hpp"
#include "engine/sound.h"
#include "movie.h"
#include "nightmare/restoration/layer2.hpp"
#include "options.h"
#include "player.h"
#include "quests.h"

namespace devilution {
namespace nightmare {

const LostContentEntry NightmareRestorationRegistry[] = {
	{
	    "tremain",
	    ContentLayer::OriginalUnused,
	    ContentStatus::FullyRestored,
	    true, true, true, false, true,
	    "towners/priest/priest8.cel",
	    "priest00.wav-priest07.wav, TCRF",
	    "Tremain the Priest in Tristram church ruins with Shadowfang quest"
	},
	{
	    "fleshdoom",
	    ContentLayer::CutContent,
	    ContentStatus::FullyRestored,
	    true, true, false, false, true,
	    "sfx/towners/priest05.wav",
	    "priest05.wav, Shadowfang item data",
	    "Fleshdoom Unique Boss in Catacombs 5-7 wielding Shadowfang"
	},
	{
	    "butcher_cinematic",
	    ContentLayer::OriginalUnused,
	    ContentStatus::FullyRestored,
	    false, false, false, true, false,
	    "gendata/fbutch3.smk",
	    "fbutch3.smk in MPQ",
	    "Butcher Chamber entrance video cinematic trigger"
	},
	{
	    "wirt_reversed",
	    ContentLayer::OriginalUnused,
	    ContentStatus::FullyRestored,
	    true, true, false, false, true,
	    "sfx/towners/pegboy21.wav",
	    "pegboy21.wav, pegboy22.wav in MPQ",
	    "Wirt reversed speech riddle (50% probability after Butcher room visit)"
	},
	{
	    "ambient_animals",
	    ContentLayer::OriginalUnused,
	    ContentStatus::FullyRestored,
	    false, false, false, false, true,
	    "sfx/animals/*",
	    "sfx/animals/*.wav in MPQ",
	    "Tristram ambient animal background sound queue"
	},
	{
	    "map_of_stars",
	    ContentLayer::OriginalUnused,
	    ContentStatus::FullyRestored,
	    true, true, false, false, true,
	    "sfx/towners/cain22.wav",
	    "TEXT_DOOM1 to TEXT_DOOM10 in MPQ",
	    "Map of the Stars prophecy lines (Cain22.wav - Cain24.wav)"
	},
	{
	    "town_gossip_lore",
	    ContentLayer::OriginalUnused,
	    ContentStatus::FullyRestored,
	    true, true, false, false, true,
	    "textdat.tsv",
	    "TEXT_FARNHAM16-22, TEXT_PEPIN12-30, TEXT_GRISWOLD14-37, TEXT_GILLIAN11-26",
	    "Tristram extended cut gossip & towners lore library"
	},
	{
	    "incinerator",
	    ContentLayer::CutContent,
	    ContentStatus::FullyRestored,
	    false, false, true, false, true,
	    "monsters/fireman/firem",
	    "MT_INCIN fireman/firem artwork & FireManAi",
	    "Incinerator monster & unique bosses (Wrathfire, Warpfire)"
	},
	{
	    "arch_lich_malignus",
	    ContentLayer::CutContent,
	    ContentStatus::FullyRestored,
	    false, false, true, false, true,
	    "monsters/darkmage/dmage",
	    "MT_DARKMAGE darkmage/dmage artwork",
	    "Arch-Lich Malignus supreme boss in level 15/16"
	}
};

const size_t NightmareRestorationRegistrySize = sizeof(NightmareRestorationRegistry) / sizeof(NightmareRestorationRegistry[0]);

namespace {
bool g_butcherCinematicPlayed = false;
std::vector<std::unique_ptr<TSnd>> g_restorationSnds;
} // namespace

void InitNightmareRestoration()
{
	g_butcherCinematicPlayed = false;
	g_restorationSnds.clear();
	InitNightmareLayer2();
}

void UpdateNightmareRestoration()
{
	// Heartbeat for ambient restoration tasks
}

bool ShouldTriggerWirtReversed50Percent()
{
	// 50% probability check as requested
	return (GenerateRnd(100) < 50);
}

void PlayNightmareRestorationWav(const char *wavPath)
{
	if (wavPath == nullptr || *wavPath == '\0')
		return;

#ifndef NOSOUND
	for (auto &snd : g_restorationSnds) {
		if (snd && snd->DSB.IsLoaded())
			snd->DSB.Stop();
	}
	g_restorationSnds.clear();

	auto sndRes = SoundFileLoadWithStatus(wavPath, /*stream=*/false);
	if (sndRes.has_value() && *sndRes != nullptr) {
		auto snd = std::move(*sndRes);
		if (snd) {
			snd_play_snd(snd.get(), 0, 0);
			g_restorationSnds.push_back(std::move(snd));
		}
	}
#endif
}

void CheckAndTriggerButcherCinematic()
{
	if (g_butcherCinematicPlayed)
		return;

	if (Quests[Q_BUTCHER]._qactive != QUEST_NOTAVAIL) {
		g_butcherCinematicPlayed = true;
		play_movie("gendata\\fbutch3.smk", /*user_can_close=*/true);
	}
}

} // namespace nightmare
} // namespace devilution
