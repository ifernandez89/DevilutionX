/**
 * @file oracle_system.h
 * 
 * 🔮 EL ORÁCULO - Sistema de Preguntas Pendientes
 * 
 * Sistema que guarda preguntas del jugador y las responde
 * en momentos seguros del juego usando Ollama local.
 * 
 * FILOSOFÍA: El Infierno responde cuando quiere, no cuando el jugador pregunta.
 */

#pragma once

#include <string>
#include <optional>
#include <cstdint>

namespace devilution {

/**
 * @brief Estado del jugador cuando hace la pregunta
 */
enum class PlayerState {
	FRIENDLY,  // Jugador en buena situación (ciudad, nivel limpio, etc.)
	ATTACK     // Jugador en peligro (murió, bajo HP, combate, etc.)
};

/**
 * @brief Pregunta pendiente del jugador
 */
struct PendingQuestion {
	std::string text;           // Pregunta del jugador (sin el '?')
	std::string context;        // Contexto: "Dungeon Level 5", "Tristram", etc.
	PlayerState state;          // Estado del jugador
	uint32_t timestamp;         // Cuándo se hizo la pregunta (SDL_GetTicks)
	bool processed;             // Ya fue respondida?
};

/**
 * @brief Sistema del Oráculo - Gestión de preguntas pendientes
 * 
 * Este sistema:
 * - Guarda preguntas del jugador localmente (sin red, sin IA)
 * - Las responde en eventos seguros (muerte, ciudad, libro, etc.)
 * - Usa Ollama local para generar respuestas crípticas
 * - Nunca bloquea el gameplay
 * - Funciona 100% normal si Ollama no está disponible
 */
class OracleSystem {
private:
	static std::optional<PendingQuestion> pendingQuestion;
	
public:
	/**
	 * @brief Añade una pregunta pendiente
	 * @param question Texto de la pregunta (sin el '?')
	 * @param context Contexto actual del juego
	 * @param state Estado del jugador
	 */
	static void AddQuestion(
		const std::string& question,
		const std::string& context = "",
		PlayerState state = PlayerState::FRIENDLY
	);
	
	/**
	 * @brief Verifica si hay una pregunta pendiente
	 * @return true si hay pregunta esperando respuesta
	 */
	static bool HasPendingQuestion();
	
	/**
	 * @brief Obtiene la pregunta pendiente
	 * @return Pregunta pendiente (debe verificar HasPendingQuestion primero)
	 */
	static PendingQuestion GetPendingQuestion();
	
	/**
	 * @brief Limpia la pregunta pendiente
	 */
	static void ClearPendingQuestion();
	
	/**
	 * @brief Marca la pregunta como procesada
	 */
	static void MarkAsProcessed();
	
	/**
	 * @brief Muestra mensaje de bienvenida del Oráculo
	 * Se llama al iniciar el juego
	 */
	static void ShowWelcomeMessage();
};

} // namespace devilution
