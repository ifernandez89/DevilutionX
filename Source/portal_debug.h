#pragma once
/**
 * @file portal_debug.h
 * @brief Portal and Level Transition Debug System
 * 
 * Sistema de debugging para detectar crashes al usar portales
 * Similar al sistema de Apocalypse crash debug
 */

#include <string>
#include <fstream>
#include <chrono>
#include <ctime>

namespace devilution {

// ============================================================================
// PORTAL DEBUG LOGGING MACROS
// ============================================================================

void InitPortalDebug();
void LogPortalEvent(const char* event, const char* details);
void LogPortalLighting(const char* phase, int lightCount, int activeLights);
void LogPortalState(const char* state, int currlevel, int leveltype);
void LogPortalGameLoop(const char* phase);
void LogPortalRender(const char* phase);
void FlushPortalDebugLog();

#define PORTAL_LOG_EVENT(event, details) LogPortalEvent(event, details)
#define PORTAL_LOG_LIGHTING(phase, lightCount, activeLights) LogPortalLighting(phase, lightCount, activeLights)
#define PORTAL_LOG_STATE(state, currlevel, leveltype) LogPortalState(state, currlevel, leveltype)
#define PORTAL_LOG_GAMELOOP(phase) LogPortalGameLoop(phase)
#define PORTAL_LOG_RENDER(phase) LogPortalRender(phase)
#define PORTAL_LOG_FLUSH() FlushPortalDebugLog()

} // namespace devilution
