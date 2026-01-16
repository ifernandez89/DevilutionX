/**
 * @file oracle_prompt.h
 * 
 * 🔮 EL ORÁCULO - Sistema de Prompts
 * 
 * Construye prompts crípticos y atmosféricos para Ollama
 * basados en el contexto del juego y estado del jugador.
 */

#pragma once

#include <string>

#include "oracle_system.h"

namespace devilution {

/**
 * @brief Constructor de prompts para el Oráculo
 */
class OraclePrompt {
public:
	/**
	 * @brief Construye un prompt completo para Ollama
	 * 
	 * @param question Pregunta del jugador
	 * @param event Evento que disparó la respuesta (PLAYER_DEATH, etc.)
	 * @param state Estado del jugador (FRIENDLY/ATTACK)
	 * @param context Contexto adicional (nivel, NPC, etc.)
	 * @return Prompt completo listo para Ollama
	 */
	static std::string BuildPrompt(
		const std::string& question,
		const std::string& event,
		PlayerState state,
		const std::string& context = ""
	);
	
private:
	/**
	 * @brief Obtiene el tono según el estado del jugador
	 * @param state Estado del jugador
	 * @return Descripción del tono
	 */
	static const char* GetTone(PlayerState state);
	
	/**
	 * @brief Obtiene contexto adicional según el evento
	 * @param event Nombre del evento
	 * @return Descripción del contexto
	 */
	static const char* GetEventContext(const std::string& event);
};

} // namespace devilution
