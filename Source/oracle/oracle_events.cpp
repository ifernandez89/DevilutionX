/**
 * @file oracle_events.cpp
 * 
 * 🔮 EL ORÁCULO - Implementación del Sistema de Eventos
 */

#include "oracle_events.h"

#include "oracle_system.h"
#include "plrmsg.h"
#include "utils/log.hpp"

namespace devilution {

// ============================================================================
// 🔮 IMPLEMENTACIÓN
// ============================================================================

void OracleEvents::TriggerEvent(OracleEvent event, const std::string& context)
{
	// 1. Verificar si hay pregunta pendiente
	if (!OracleSystem::HasPendingQuestion()) {
#ifdef _DEBUG
		LogVerbose("Oracle: Event {} triggered but no pending question", EventToString(event));
#endif
		return;
	}
	
	// 2. Verificar si el evento es seguro
	if (!IsEventSafe(event)) {
#ifdef _DEBUG
		LogVerbose("Oracle: Event {} is not safe, ignoring", EventToString(event));
#endif
		return;
	}
	
	// 3. Obtener pregunta pendiente
	PendingQuestion question = OracleSystem::GetPendingQuestion();
	
#ifdef _DEBUG
	LogVerbose("Oracle: Event {} triggered with question: \"{}\"", 
		EventToString(event), question.text);
#endif
	
	// 4. TODO (Paso 5): Verificar si Ollama está disponible
	// if (!OllamaClient::IsAvailable()) {
	//     OracleSystem::ClearPendingQuestion();
	//     return;
	// }
	
	// 5. TODO (Paso 5): Verificar cache
	// auto cachedResponse = OracleCache::GetResponse(question.text);
	// if (cachedResponse.has_value()) {
	//     DisplayOracleResponse(*cachedResponse);
	//     OracleSystem::ClearPendingQuestion();
	//     return;
	// }
	
	// 6. TODO (Paso 6): Construir prompt y llamar Ollama
	// std::string prompt = OraclePrompt::BuildPrompt(
	//     question.text,
	//     EventToString(event),
	//     question.state == PlayerState::FRIENDLY ? "FRIENDLY" : "ATTACK",
	//     context
	// );
	
	// 7. TODO (Paso 6): Query asíncrono a Ollama
	// OllamaClient::QueryAsync(request, callback);
	
	// 8. Por ahora, mostrar mensaje de placeholder
	EventPlrMsg("🔮 EL ORÁCULO MEDITA TU PREGUNTA...", UiFlags::ColorRed);
	EventPlrMsg("    (Sistema de respuestas en desarrollo - Paso 5/6)", UiFlags::ColorWhitegold);
	
	// Limpiar pregunta (por ahora)
	OracleSystem::ClearPendingQuestion();
}

bool OracleEvents::IsEventSafe(OracleEvent event)
{
	// Todos los eventos definidos son seguros
	// (muerte, ciudad, libros, altares, NPCs)
	switch (event) {
		case OracleEvent::PLAYER_DEATH:
		case OracleEvent::LEVEL_CLEARED:
		case OracleEvent::ENTERED_TOWN:
		case OracleEvent::BOOK_INTERACTION:
		case OracleEvent::ALTAR_INTERACTION:
		case OracleEvent::NPC_INTERACTION:
			return true;
		default:
			return false;
	}
}

const char* OracleEvents::EventToString(OracleEvent event)
{
	switch (event) {
		case OracleEvent::PLAYER_DEATH:
			return "PLAYER_DEATH";
		case OracleEvent::LEVEL_CLEARED:
			return "LEVEL_CLEARED";
		case OracleEvent::ENTERED_TOWN:
			return "ENTERED_TOWN";
		case OracleEvent::BOOK_INTERACTION:
			return "BOOK_INTERACTION";
		case OracleEvent::ALTAR_INTERACTION:
			return "ALTAR_INTERACTION";
		case OracleEvent::NPC_INTERACTION:
			return "NPC_INTERACTION";
		default:
			return "UNKNOWN";
	}
}

} // namespace devilution
