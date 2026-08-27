#include "nightmare/restoration/registry.hpp"

#include <memory>

#include "engine/random.hpp"
#include "engine/sound.h"
#include "movie.h"
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
	for (auto it = g_restorationSnds.begin(); it != g_restorationSnds.end();) {
		if (!*it || !(*it)->DSB.IsPlaying()) {
			it = g_restorationSnds.erase(it);
		} else {
			++it;
		}
	}

	auto sndRes = SoundFileLoadWithStatus(wavPath, /*stream=*/false);
	if (sndRes.has_value() && *sndRes != nullptr) {
		auto snd = std::move(*sndRes);
		if (snd) {
			snd->DSB.Play(*GetOptions().Audio.soundVolume);
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
