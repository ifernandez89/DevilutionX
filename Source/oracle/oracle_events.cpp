/**
 * @file oracle_events.cpp
 * 
 * 🔮 EL ORÁCULO - Implementación del Sistema de Eventos
 */

#include "oracle_events.h"

#include <mutex>

#include "oracle_system.h"
#include "oracle_ollama.h"
#include "oracle_prompt.h"
#include "plrmsg.h"
#include "utils/log.hpp"

namespace devilution {

// Mutex para thread safety (callback de Ollama se ejecuta en otro thread)
std::mutex g_oracleMutex;

} // namespace devilution

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
	
	// 4. Verificar si Ollama está disponible
	if (!OracleOllama::IsAvailable()) {
#ifdef _DEBUG
		LogVerbose("Oracle: Ollama not available, clearing question");
#endif
		OracleSystem::ClearPendingQuestion();
		return;
	}
	
	// 5. TODO (Paso 7): Verificar cache
	// auto cachedResponse = OracleCache::GetResponse(question.text);
	// if (cachedResponse.has_value()) {
	//     EventPlrMsg(StrCat("🔮 ", *cachedResponse), UiFlags::ColorRed);
	//     OracleSystem::ClearPendingQuestion();
	//     return;
	// }
	
	// 6. Construir prompt
	std::string prompt = OraclePrompt::BuildPrompt(
		question.text,
		EventToString(event),
		question.state,
		context
	);
	
#ifdef _DEBUG
	LogVerbose("Oracle: Querying Ollama...");
#endif
	
	// Mostrar indicador visual
	EventPlrMsg("🔮 El Oráculo medita tu pregunta...", UiFlags::ColorRed);
	
	// 7. Query asíncrono a Ollama
	OracleOllama::QueryAsync(prompt, [](std::optional<std::string> response) {
		// IMPORTANTE: Este callback se ejecuta en otro thread
		// Usar mutex para acceso seguro
		std::lock_guard<std::mutex> lock(g_oracleMutex);
		
		if (response.has_value()) {
			// Mostrar respuesta del Oráculo
			EventPlrMsg(StrCat("🔮 ", *response), UiFlags::ColorRed);
			
#ifdef _DEBUG
			LogVerbose("Oracle: Response displayed");
#endif
			
			// TODO (Paso 7): Guardar en cache
			// OracleCache::SaveResponse(question.text, *response);
		} else {
#ifdef _DEBUG
			LogVerbose("Oracle: No response from Ollama");
#endif
		}
		
		// Limpiar pregunta pendiente
		OracleSystem::ClearPendingQuestion();
	});
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
