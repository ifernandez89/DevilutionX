/**
 * @file portal_debug.cpp
 * @brief Portal and Level Transition Debug System Implementation
 */

#include "portal_debug.h"
#include "lighting.h"
#include "nightmare_lighting.h"
#include <iomanip>
#include <sstream>
#include <filesystem>

namespace devilution {

namespace {
std::ofstream g_portalDebugLog;
bool g_portalDebugInitialized = false;
int g_portalEventCount = 0;
}

void InitPortalDebug()
{
	if (g_portalDebugInitialized) {
		return;
	}
	
	// Create debug_logs directory if it doesn't exist
	std::filesystem::create_directories("debug_logs");
	
	// Generate filename with timestamp
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
	
	std::stringstream filename;
	filename << "debug_logs/portal_debug_";
	filename << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
	filename << "_" << ms.count() << ".log";
	
	g_portalDebugLog.open(filename.str(), std::ios::out | std::ios::trunc);
	
	if (g_portalDebugLog.is_open()) {
		g_portalDebugLog << "=== PORTAL DEBUG SYSTEM ===" << std::endl;
		g_portalDebugLog << "Initialized at: " << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << std::endl;
		g_portalDebugLog << "Purpose: Debug crashes when using portals" << std::endl;
		g_portalDebugLog << "Focus: Lighting system state during level transitions" << std::endl;
		g_portalDebugLog << "==============================" << std::endl << std::endl;
		g_portalDebugLog.flush();
		g_portalDebugInitialized = true;
	}
}

void LogPortalEvent(const char* event, const char* details)
{
	if (!g_portalDebugInitialized) {
		InitPortalDebug();
	}
	
	if (!g_portalDebugLog.is_open()) {
		return;
	}
	
	g_portalEventCount++;
	
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	
	g_portalDebugLog << std::put_time(std::localtime(&time), "%H:%M:%S");
	g_portalDebugLog << " [EVENT#" << g_portalEventCount << "] ";
	g_portalDebugLog << event << " - " << details << std::endl;
	g_portalDebugLog.flush();
}

void LogPortalLighting(const char* phase, int lightCount, int activeLights)
{
	if (!g_portalDebugInitialized) {
		InitPortalDebug();
	}
	
	if (!g_portalDebugLog.is_open()) {
		return;
	}
	
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	
	g_portalDebugLog << std::put_time(std::localtime(&time), "%H:%M:%S");
	g_portalDebugLog << " [LIGHTING] Phase:" << phase;
	g_portalDebugLog << " LightCount:" << lightCount;
	g_portalDebugLog << " ActiveLights:" << activeLights;
	g_portalDebugLog << " NightmareLights:" << g_nightmareLighting.activeLightCount;
	g_portalDebugLog << " Initialized:" << (g_nightmareLighting.initialized ? "YES" : "NO");
	g_portalDebugLog << std::endl;
	g_portalDebugLog.flush();
}

void LogPortalState(const char* state, int currlevel, int leveltype)
{
	if (!g_portalDebugInitialized) {
		InitPortalDebug();
	}
	
	if (!g_portalDebugLog.is_open()) {
		return;
	}
	
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	
	const char* levelTypeName = "UNKNOWN";
	switch (leveltype) {
		case 0: levelTypeName = "TOWN"; break;
		case 1: levelTypeName = "CATHEDRAL"; break;
		case 2: levelTypeName = "CATACOMBS"; break;
		case 3: levelTypeName = "CAVES"; break;
		case 4: levelTypeName = "HELL"; break;
		case 5: levelTypeName = "NEST"; break;
		case 6: levelTypeName = "CRYPT"; break;
	}
	
	g_portalDebugLog << std::put_time(std::localtime(&time), "%H:%M:%S");
	g_portalDebugLog << " [STATE] " << state;
	g_portalDebugLog << " Level:" << currlevel;
	g_portalDebugLog << " Type:" << levelTypeName << "(" << leveltype << ")";
	g_portalDebugLog << std::endl;
	g_portalDebugLog.flush();
}

void LogPortalGameLoop(const char* phase)
{
	if (!g_portalDebugInitialized) {
		InitPortalDebug();
	}
	
	if (!g_portalDebugLog.is_open()) {
		return;
	}
	
	// Solo loguear los primeros 50 eventos del game loop para no saturar
	static int gameLoopEventCount = 0;
	if (gameLoopEventCount > 50) {
		return;
	}
	gameLoopEventCount++;
	
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	
	g_portalDebugLog << std::put_time(std::localtime(&time), "%H:%M:%S");
	g_portalDebugLog << " [GAMELOOP#" << gameLoopEventCount << "] " << phase << std::endl;
	g_portalDebugLog.flush();
}

void LogPortalRender(const char* phase)
{
	if (!g_portalDebugInitialized) {
		InitPortalDebug();
	}
	
	if (!g_portalDebugLog.is_open()) {
		return;
	}
	
	// Loguear todos los eventos de render (sin límite para debugging de portal)
	static int renderEventCount = 0;
	renderEventCount++;
	
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	
	g_portalDebugLog << std::put_time(std::localtime(&time), "%H:%M:%S");
	g_portalDebugLog << " [RENDER#" << renderEventCount << "] " << phase << std::endl;
	g_portalDebugLog.flush();
}

void FlushPortalDebugLog()
{
	if (g_portalDebugLog.is_open()) {
		g_portalDebugLog.flush();
	}
}

} // namespace devilution
