#pragma once

/**
 * @file debug_hellfire_recovery.h
 * 
 * 🔍 DEBUG: Sistema de debugging para Hellfire recovery
 */

#include "player.h"

namespace devilution {

void DebugPrintAllPlayerItems(const Player& player, const std::string& context);
void DebugSearchForInfernoAnywhere();
void DebugSaveGameFlags();
void DebugTestInfernoSpellCreation();
void DebugFullDiagnostics();

} // namespace devilution