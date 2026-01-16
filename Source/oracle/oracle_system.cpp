/**
 * @file oracle_system.cpp
 * 
 * 🔮 EL ORÁCULO - Implementación del Sistema de Preguntas Pendientes
 */

#include "oracle_system.h"

#include <SDL.h>

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

} // namespace devilution
