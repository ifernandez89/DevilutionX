/**
 * @file level_atmosphere.hpp
 *
 * NIGHTMARE Level Atmospheric and Gameplay Configuration.
 * Manages procedural identity per dungeon level (lighting, music, object density,
 * monster level ranges, and thematic elements) while preserving vanilla fallbacks.
 */
#pragma once

#include <cstdint>
#include "engine/sound.h"
#include "levels/gendung_defs.hpp"

namespace devilution {
namespace nightmare {

/**
 * @brief Returns the base player light radius (in tiles) for the given dungeon level.
 * Vanilla baseline was static 12 for all levels.
 */
int GetNightmareBaseLightRadius(uint8_t level);

/**
 * @brief Returns the atmospheric music track for the given level and dungeon type.
 * Vanilla mapped purely by dungeon_type.
 */
_music_id GetNightmareLevelMusic(dungeon_type dungeonType, uint8_t level, bool isSetLevel);

/**
 * @brief Provides level-specific chest generation counts.
 * Vanilla baseline was Chest1(5,10), Chest2(3,6), Chest3(1,5).
 */
void GetNightmareChestCounts(uint8_t level, int &c1min, int &c1max, int &c2min, int &c2max, int &c3min, int &c3max);

/**
 * @brief Applies NIGHTMARE monster spawn depth (minDunLvl / maxDunLvl) overrides
 * to break player muscle memory without corrupting game tables.
 */
void ApplyNightmareMonsterLevelOverrides();

} // namespace nightmare
} // namespace devilution
