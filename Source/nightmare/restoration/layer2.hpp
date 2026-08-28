#pragma once

#include <cstddef>
#include <cstdint>

#include "tables/textdat.h"

namespace devilution {
namespace nightmare {

void InitNightmareLayer2();

/**
 * @brief Checks if a specific asset file (CEL/CLX/WAV) exists in the MPQ/VFS.
 */
bool IsLayer2AssetAvailable(const char *assetPath);

/**
 * @brief Checks if Incinerator monster graphics are present.
 */
bool IsIncineratorAssetAvailable();

/**
 * @brief Checks if Arch-Lich Malignus graphics are present.
 */
bool IsDarkMageAssetAvailable();

/**
 * @brief Safely attempts audio playback for Layer 2 dialogues without crashing if missing.
 */
void PlayLayer2WavDefensive(const char *wavPath);

/**
 * @brief Selects extended cut gossip line with 30% probability.
 */
_speech_id GetLayer2GossipDefensive(_speech_id defaultSpeech, int startCutId, int endCutId);

} // namespace nightmare
} // namespace devilution
