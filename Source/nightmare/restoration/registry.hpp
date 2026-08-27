#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace devilution {
namespace nightmare {

enum class ContentLayer {
	Original,          // Present and used in Diablo/Hellfire
	OriginalUnused,    // Present in MPQ but dormant
	HellfireOnly,      // Exclusive to Hellfire
	CutContent,        // Documented cut content
	NightmareRestored, // Activated by NIGHTMARE using original assets
	NightmareOriginal  // Created by NIGHTMARE
};

enum class ContentStatus {
	Dormant,
	PartiallyActive,
	FullyRestored,
	Reconstructed
};

struct LostContentEntry {
	const char *id;
	ContentLayer layer;
	ContentStatus status;
	bool hasDialogue;
	bool hasVoice;
	bool hasSprite;
	bool hasCinematic;
	bool hasAudio;
	const char *mpqPath;
	const char *evidence;
	const char *description;
};

extern const LostContentEntry NightmareRestorationRegistry[];
extern const size_t NightmareRestorationRegistrySize;

void InitNightmareRestoration();
void UpdateNightmareRestoration();

// Utility helpers for dormant content activation
bool ShouldTriggerWirtReversed50Percent();
void PlayNightmareRestorationWav(const char *wavPath);
void CheckAndTriggerButcherCinematic();

} // namespace nightmare
} // namespace devilution
