/**
 * @file dormant_assets_DISABLED.h
 * @brief SISTEMA DESACTIVADO TEMPORALMENTE PARA DEBUG
 * 
 * Este sistema está desactivado para encontrar la causa del crash.
 */

#pragma once

#include <string>
#include "engine/point.hpp"

namespace devilution {

// Stubs vacíos - no hacen nada
inline void InitDormantAssets() {}
inline void EnhanceLevelWithDormantAssets(int levelType, int enhancementChance = 15) {}
inline std::string GetEnhancedMonsterVariant(int originalMonsterType) { return ""; }
inline bool AddDormantVisualEffect(const std::string& effectType, Point position) { return false; }

} // namespace devilution
