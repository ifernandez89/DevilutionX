/**
 * @file hidden_content_DISABLED.h
 * @brief SISTEMA DESACTIVADO TEMPORALMENTE PARA DEBUG
 * 
 * Este sistema está desactivado para encontrar la causa del crash.
 */

#pragma once

#include "tables/textdat.h"

namespace devilution {

// Stubs vacíos - no hacen nada
inline void InitHiddenContent() {}
inline _speech_id GetRandomHiddenGossip(_speech_id fallback) { return fallback; }
inline _speech_id GetRandomHiddenBookText(_speech_id fallback) { return fallback; }

} // namespace devilution
