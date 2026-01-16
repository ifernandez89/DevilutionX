/**
 * @file oracle_events.h
 * 
 * 🔮 EL ORÁCULO - Sistema de Detección de Eventos Seguros
 * 
 * Detecta momentos seguros del juego donde el Oráculo puede responder
 * sin interrumpir el gameplay.
 */

#pragma once

#include <string>

namespace devilution {

/**
 * @brief Eventos seguros donde el Oráculo puede responder
 */
enum class OracleEvent {
	PLAYER_DEATH,       // ☠️ Jugador murió
	LEVEL_CLEARED,      // 🧹 Nivel limpiado (todos los monstruos muertos)
	ENTERED_TOWN,       // 🏘️ Entró a ciudad (Tristram)
	BOOK_INTERACTION,   // 📖 Leyó un libro
	ALTAR_INTERACTION,  // 🕯️ Usó un altar
	NPC_INTERACTION     // 💬 Habló con un NPC
};

/**
 * @brief Sistema de Eventos del Oráculo
 * 
 * Detecta eventos seguros y dispara respuestas del Oráculo
 * cuando hay preguntas pendientes.
 */
class OracleEvents {
public:
	/**
	 * @brief Dispara un evento del Oráculo
	 * 
	 * Si hay pregunta pendiente y Ollama disponible,
	 * genera respuesta críptica.
	 * 
	 * @param event Tipo de evento
	 * @param context Contexto adicional (nivel, NPC, etc.)
	 */
	static void TriggerEvent(OracleEvent event, const std::string& context = "");
	
	/**
	 * @brief Verifica si un evento es seguro para responder
	 * @param event Tipo de evento
	 * @return true si es seguro responder en este evento
	 */
	static bool IsEventSafe(OracleEvent event);
	
	/**
	 * @brief Convierte evento a string (para logging)
	 * @param event Tipo de evento
	 * @return Nombre del evento
	 */
	static const char* EventToString(OracleEvent event);
};

} // namespace devilution
