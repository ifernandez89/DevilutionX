/**
 * @file oracle_system.cpp
 * 
 * 🔮 EL ORÁCULO - Implementación del Sistema de Preguntas Pendientes
 */

#include "oracle_system.h"

#include <SDL.h>

#include "plrmsg.h"  // Para EventPlrMsg
#include "utils/log.hpp"

namespace devilution {

// ============================================================================
// 🔮 ESTADO GLOBAL
// ============================================================================

std::optional<PendingQuestion> OracleSystem::pendingQuestion = std::nullopt;

// ============================================================================
// 🔮 IMPLEMENTACIÓN
// ============================================================================

void OracleSystem::AddQuestion(
    const std::string& question,
    const std::string& context,
    PlayerState state
)
{
	// Validación básica
	if (question.empty() || question.length() < 3) {
		return;  // Pregunta demasiado corta
	}
	
	// Si ya hay una pregunta pendiente, la reemplazamos
	// (el jugador puede cambiar de opinión)
	pendingQuestion = PendingQuestion{
		question,
		context,
		state,
		SDL_GetTicks(),
		false
	};
	
#ifdef _DEBUG
	LogVerbose("Oracle: Question added - \"{}\" (context: {}, state: {})",
		question,
		context.empty() ? "none" : context,
		state == PlayerState::FRIENDLY ? "FRIENDLY" : "ATTACK"
	);
#endif
}

bool OracleSystem::HasPendingQuestion()
{
	return pendingQuestion.has_value() && !pendingQuestion->processed;
}

PendingQuestion OracleSystem::GetPendingQuestion()
{
	// SAFETY: Caller debe verificar HasPendingQuestion() primero
	if (!pendingQuestion.has_value()) {
		// Retornar pregunta vacía como fallback
		return PendingQuestion{
			"",
			"",
			PlayerState::FRIENDLY,
			0,
			true
		};
	}
	
	return *pendingQuestion;
}

void OracleSystem::ClearPendingQuestion()
{
	pendingQuestion = std::nullopt;
	
#ifdef _DEBUG
	LogVerbose("Oracle: Question cleared");
#endif
}

void OracleSystem::MarkAsProcessed()
{
	if (pendingQuestion.has_value()) {
		pendingQuestion->processed = true;
		
#ifdef _DEBUG
		LogVerbose("Oracle: Question marked as processed");
#endif
	}
}

void OracleSystem::ShowWelcomeMessage()
{
	// Mensajes de bienvenida crípticos del Oráculo
	// Se elige uno al azar cada vez que se inicia el juego
	static const char* welcomeMessages[] = {
		"🔮 El Infierno te observa, mortal. Tus pasos resuenan en la oscuridad.",
		"🔮 Bienvenido a la pesadilla. El Oráculo aguarda tus preguntas... y tu caída.",
		"🔮 Las sombras susurran tu nombre. El destino ya está escrito.",
		"🔮 Otro alma perdida cruza el umbral. El Infierno no olvida, no perdona.",
		"🔮 La luz se desvanece. Solo la oscuridad y el Oráculo permanecen.",
		"🔮 Tus preguntas serán escuchadas. Tus respuestas, temidas.",
		"🔮 El abismo te contempla. El Oráculo habla cuando la muerte acecha.",
		"🔮 Bienvenido, viajero. El Infierno tiene mucho que enseñarte.",
		"🔮 Las runas antiguas brillan. El Oráculo despierta de su letargo.",
		"🔮 Otro condenado busca respuestas. El Infierno solo ofrece verdades crueles."
	};
	
	// Seleccionar mensaje aleatorio
	const int messageCount = sizeof(welcomeMessages) / sizeof(welcomeMessages[0]);
	const int index = SDL_GetTicks() % messageCount;
	
	// Mostrar mensaje de bienvenida
	EventPlrMsg(welcomeMessages[index], UiFlags::ColorRed);
	EventPlrMsg("    Escribe en el chat y el Oráculo responderá en el momento oportuno.", UiFlags::ColorWhitegold);
	
#ifdef _DEBUG
	LogVerbose("Oracle: Welcome message shown (index: {})", index);
#endif
}

} // namespace devilution
