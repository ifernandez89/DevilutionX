/**
 * @file oracle_ui.cpp
 * 
 * 🔮 EL ORÁCULO - Implementación del Sistema de UI
 */

#include "oracle_ui.h"

#include <SDL.h>

#include "control/control.hpp"
#include "engine/backbuffer_state.hpp"
#include "engine/dx.h"
#include "engine/render/text_render.hpp"
#include "utils/log.hpp"

namespace devilution {

// ============================================================================
// 🔮 VARIABLES GLOBALES
// ============================================================================

OracleUIMessage OracleUI::chatMessage;
OracleUIMessage OracleUI::debugMessage;

// ============================================================================
// 🔮 CONFIGURACIÓN
// ============================================================================

constexpr uint32_t CHAT_MESSAGE_DURATION = 5000;   // 5 segundos
constexpr uint32_t DEBUG_MESSAGE_DURATION = 4000;  // 4 segundos
constexpr uint32_t FADE_IN_DURATION = 1000;        // 1 segundo
constexpr uint32_t FADE_OUT_DURATION = 1000;       // 1 segundo

// Posición del mensaje de debug
constexpr int DEBUG_MESSAGE_X = 10;
constexpr int DEBUG_MESSAGE_Y = 50;

// ============================================================================
// 🔮 ORACLE UI MESSAGE
// ============================================================================

void OracleUIMessage::Show(const std::string& message, uint32_t durationMs)
{
	text = message;
	startTime = SDL_GetTicks();
	duration = durationMs;
	visible = true;
	alpha = 0.0f;
	
#ifdef _DEBUG
	LogVerbose("Oracle UI: Showing message: \"{}\" ({}ms)", message, durationMs);
#endif
}

void OracleUIMessage::Update()
{
	if (!visible) {
		alpha = 0.0f;
		return;
	}
	
	uint32_t now = SDL_GetTicks();
	uint32_t elapsed = now - startTime;
	
	if (elapsed < FADE_IN_DURATION) {
		// Fade in (0-1 segundo)
		alpha = static_cast<float>(elapsed) / static_cast<float>(FADE_IN_DURATION);
	} else if (elapsed < duration - FADE_OUT_DURATION) {
		// Visible (1-4 segundos)
		alpha = 1.0f;
	} else if (elapsed < duration) {
		// Fade out (4-5 segundos)
		uint32_t fadeElapsed = elapsed - (duration - FADE_OUT_DURATION);
		alpha = 1.0f - (static_cast<float>(fadeElapsed) / static_cast<float>(FADE_OUT_DURATION));
	} else {
		// Desaparecer
		Hide();
	}
}

void OracleUIMessage::Hide()
{
	visible = false;
	alpha = 0.0f;
	text.clear();
}

// ============================================================================
// 🔮 ORACLE UI
// ============================================================================

void OracleUI::ShowMessage(const std::string& message)
{
	if (message.empty()) {
		return;
	}
	
	// Routing inteligente
	if (IsChatActive()) {
		// CANAL A: Chat voice line
		chatMessage.Show(message, CHAT_MESSAGE_DURATION);
#ifdef _DEBUG
		LogVerbose("Oracle UI: Routing to CHAT (Canal A)");
#endif
	} else {
		// CANAL B: Debug overlay
		debugMessage.Show(message, DEBUG_MESSAGE_DURATION);
#ifdef _DEBUG
		LogVerbose("Oracle UI: Routing to DEBUG (Canal B)");
#endif
	}
}

void OracleUI::Update()
{
	chatMessage.Update();
	debugMessage.Update();
}

void OracleUI::RenderChatMessage()
{
	if (!chatMessage.IsVisible()) {
		return;
	}
	
	// Renderizar en la parte superior del chat como "voice line"
	// Posición: arriba del panel principal, centrado
	const Point mainPanelPosition = GetMainPanel().position;
	const int x = mainPanelPosition.x + 320; // Centro del panel (640/2)
	const int y = mainPanelPosition.y - 20;  // 20 píxeles arriba del panel
	
	const Surface &out = GlobalBackBuffer();
	
	// Color blanco centrado
	UiFlags flags = UiFlags::ColorWhite | UiFlags::AlignCenter | UiFlags::FontSize12;
	
	// Renderizar el texto centrado
	DrawString(out, chatMessage.text, { { x - 200, y }, { 400, 20 } }, { .flags = flags });
}

void OracleUI::RenderDebugMessage()
{
	if (!debugMessage.IsVisible()) {
		return;
	}
	
	// Renderizar en debug overlay (visible sin F12)
	// Posición: esquina superior izquierda, debajo de FPS
	const Surface &out = GlobalBackBuffer();
	
	// Color blanco
	UiFlags flags = UiFlags::ColorWhite | UiFlags::FontSize12;
	
	// Renderizar el texto
	DrawString(out, debugMessage.text, { { DEBUG_MESSAGE_X, DEBUG_MESSAGE_Y }, { 600, 20 } }, { .flags = flags });
}

bool OracleUI::HasVisibleMessage()
{
	return chatMessage.IsVisible() || debugMessage.IsVisible();
}

bool OracleUI::IsChatActive()
{
	// Verificar si el chat está abierto
	return ChatFlag;
}

} // namespace devilution
