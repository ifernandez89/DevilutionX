/**
 * @file nightmare_items_DISABLED.h
 * @brief SISTEMA DESACTIVADO TEMPORALMENTE PARA DEBUG
 * 
 * Este sistema está desactivado para encontrar la causa del crash.
 * Todos los macros retornan false/disabled.
 */

#pragma once

// 🚫 DESACTIVADO COMPLETAMENTE
#define NIGHTMARE_ITEMS_ENABLED() (false)
#define NIGHTMARE_ITEMS_SPAWN_RATE() (0.0f)
#define IS_NIGHTMARE_ITEM(x) (false)

namespace devilution {

// Stubs vacíos - no hacen nada
inline void InitNightmareItems() {}
inline void UpdateNightmareItems() {}
inline void CleanupNightmareItems() {}
inline bool GenerateNightmareItem(int itemId, int x, int y) { return false; }
inline bool GenerateRandomNightmareItem(int x, int y, int minLevel) { return false; }

} // namespace devilution
