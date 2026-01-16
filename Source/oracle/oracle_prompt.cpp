/**
 * @file oracle_prompt.cpp
 * 
 * 🔮 EL ORÁCULO - Implementación del Sistema de Prompts
 */

#include "oracle_prompt.h"

#include "utils/str_cat.hpp"

namespace devilution {

// ============================================================================
// 🔮 PROMPT MAESTRO
// ============================================================================

constexpr const char* MASTER_PROMPT = R"(You are an ancient, cryptic entity from Diablo's dark world - the Oracle of the Inferno.

The player asks: "{QUESTION}"

Context:
- Event: {EVENT}
- Mood: {TONE}
- Location: {CONTEXT}

Respond as the Oracle:
- Speak in dark, poetic, cryptic language
- Maximum 3 short lines
- Use metaphors of darkness, death, and fate
- No modern terms, no tutorials, no game mechanics
- Sound like an ancient prophecy or dark whisper
- Be ominous but not directly helpful

Example style:
"The shadows know your name, mortal. They whisper of your fall.
Each death carves your path deeper into the abyss.
The Inferno watches... and waits."

Now respond to the player's question:)";

// ============================================================================
// 🔮 IMPLEMENTACIÓN
// ============================================================================

std::string OraclePrompt::BuildPrompt(
	const std::string& question,
	const std::string& event,
	PlayerState state,
	const std::string& context
)
{
	// Obtener tono según estado
	const char* tone = GetTone(state);
	
	// Obtener contexto del evento
	const char* eventContext = GetEventContext(event);
	
	// Construir contexto completo
	std::string fullContext = context.empty() ? eventContext : StrCat(eventContext, " - ", context);
	
	// Reemplazar placeholders en el prompt maestro
	std::string prompt = MASTER_PROMPT;
	
	// Reemplazar {QUESTION}
	size_t pos = prompt.find("{QUESTION}");
	if (pos != std::string::npos) {
		prompt.replace(pos, 10, question);
	}
	
	// Reemplazar {EVENT}
	pos = prompt.find("{EVENT}");
	if (pos != std::string::npos) {
		prompt.replace(pos, 7, event);
	}
	
	// Reemplazar {TONE}
	pos = prompt.find("{TONE}");
	if (pos != std::string::npos) {
		prompt.replace(pos, 6, tone);
	}
	
	// Reemplazar {CONTEXT}
	pos = prompt.find("{CONTEXT}");
	if (pos != std::string::npos) {
		prompt.replace(pos, 9, fullContext);
	}
	
	return prompt;
}

const char* OraclePrompt::GetTone(PlayerState state)
{
	switch (state) {
		case PlayerState::FRIENDLY:
			return "Calm, contemplative, mysterious";
		case PlayerState::ATTACK:
			return "Ominous, threatening, dark";
		default:
			return "Neutral, cryptic";
	}
}

const char* OraclePrompt::GetEventContext(const std::string& event)
{
	if (event == "PLAYER_DEATH") {
		return "The player has fallen in battle";
	}
	if (event == "ENTERED_TOWN") {
		return "The player seeks refuge in Tristram";
	}
	if (event == "LEVEL_CLEARED") {
		return "The player has vanquished all foes";
	}
	if (event == "BOOK_INTERACTION") {
		return "The player reads ancient knowledge";
	}
	if (event == "ALTAR_INTERACTION") {
		return "The player communes with dark powers";
	}
	if (event == "NPC_INTERACTION") {
		return "The player speaks with a denizen of Tristram";
	}
	
	return "Unknown circumstances";
}

} // namespace devilution
