/**
 * @file ui_integration.cpp
 * @brief Nightmare Edition - UI Integration Implementation
 * 
 * IMPLEMENTACIÓN DE INTEGRACIÓN UI
 * ===============================
 * 
 * Sistema que conecta los efectos contemplativos con
 * los elementos UI existentes de DevilutionX.
 */

#include "ui_integration.h"

#include "ui_contemplative.h"
#include "utils/log.hpp"

namespace devilution {

// Mapeo de elementos de menú a elementos contemplativos
namespace {
	// IDs de elementos de menú principal
	constexpr int MAIN_MENU_SINGLE_PLAYER = 0;
	constexpr int MAIN_MENU_MULTIPLAYER = 1;
	constexpr int MAIN_MENU_REPLAY = 2;
	constexpr int MAIN_MENU_SETTINGS = 3;
	constexpr int MAIN_MENU_CREDITS = 4;
	constexpr int MAIN_MENU_EXIT = 5;
	
	// IDs de elementos de menú de juego
	constexpr int GAME_MENU_SAVE_EXIT = 0;
	constexpr int GAME_MENU_RETURN_GAME = 1;
	constexpr int GAME_MENU_OPTIONS = 2;
	constexpr int GAME_MENU_NEW_GAME = 3;
	constexpr int GAME_MENU_LOAD_GAME = 4;
	constexpr int GAME_MENU_EXIT_DIABLO = 5;
}

void InitNightmareUIIntegration()
{
	LogVerbose("Initializing Nightmare UI Integration");
	LogVerbose("🌙 Contemplative effects ready for menu integration");
}

void HandleMainMenuHover(int menuItem)
{
	// Aplicar efecto contemplativo al elemento de menú principal
	HandleContemplativeHover(contemplativeUI.mainMenuButtons);
	
	LogVerbose("🫁 Main menu hover: item {}", menuItem);
}

void HandleMainMenuUnhover(int menuItem)
{
	// Remover efecto contemplativo del elemento de menú principal
	HandleContemplativeUnhover(contemplativeUI.mainMenuButtons);
	
	LogVerbose("🫁 Main menu unhover: item {}", menuItem);
}

bool HandleMainMenuClick(int menuItem)
{
	// Aplicar delay contemplativo al click del menú principal
	bool shouldProcessImmediately = HandleContemplativeClick(contemplativeUI.mainMenuButtons);
	
	LogVerbose("⏳ Main menu click: item {} (delay: {})", 
		menuItem, shouldProcessImmediately ? "none" : "150ms");
	
	return shouldProcessImmediately;
}

void HandleGameMenuHover(int menuItem)
{
	// Aplicar efecto contemplativo al elemento de menú de juego
	HandleContemplativeHover(contemplativeUI.gameMenuButtons);
	
	LogVerbose("🫁 Game menu hover: item {}", menuItem);
}

void HandleGameMenuUnhover(int menuItem)
{
	// Remover efecto contemplativo del elemento de menú de juego
	HandleContemplativeUnhover(contemplativeUI.gameMenuButtons);
	
	LogVerbose("🫁 Game menu unhover: item {}", menuItem);
}

bool HandleGameMenuClick(int menuItem)
{
	// Aplicar delay contemplativo al click del menú de juego
	bool shouldProcessImmediately = HandleContemplativeClick(contemplativeUI.gameMenuButtons);
	
	LogVerbose("⏳ Game menu click: item {} (delay: {})", 
		menuItem, shouldProcessImmediately ? "none" : "150ms");
	
	return shouldProcessImmediately;
}

int GetMenuRenderOffset(int menuItem)
{
	// Obtener offset de breathing para el elemento de menú
	// Por simplicidad, usar el mismo elemento contemplativo para todos los items
	return GetContemplativeRenderOffset(contemplativeUI.mainMenuButtons);
}

float GetMenuRenderAlpha(int menuItem)
{
	// Obtener alpha contemplativo para el elemento de menú
	return GetContemplativeAlpha(contemplativeUI.mainMenuButtons);
}

float GetMenuGlowIntensity(int menuItem)
{
	// Obtener intensidad de glow para el elemento de menú
	return GetContemplativeGlow(contemplativeUI.mainMenuButtons);
}

bool IsMenuInContemplativeDelay(int menuItem)
{
	// Verificar si el elemento está en delay contemplativo
	return IsInContemplativeDelay(contemplativeUI.mainMenuButtons);
}

} // namespace devilution