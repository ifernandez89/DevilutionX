/**
 * @file oracle_prompt.cpp
 * 
 * 🔮 EL ORÁCULO - Implementación del Sistema de Prompts
 * 
 * FILOSOFÍA CRÍTICA:
 * La IA NO genera contenido libre. La IA SOLO hace micro-variaciones
 * de textos originales/dormidos de Diablo.
 */

#include "oracle_prompt.h"
#include "oracle_dormant_texts.h"

#include <vector>

#include "utils/str_cat.hpp"

namespace devilution {

// ============================================================================
// 🔮 PROMPT MAESTRO CON TEXTOS DORMIDOS
// ============================================================================

constexpr const char* MASTER_PROMPT_WITH_DORMANT = R"(You are the voice of the dungeon itself from Diablo (1996).

CRITICAL RULES:
===============
- You are NOT a narrator or guide
- You are the world speaking through darkness
- ABSORB the meaning from examples below
- REINTERPRET the idea in your own words
- Do NOT rephrase or echo original sentences
- Do NOT give advice or instructions
- Speak in abstract, fatalistic terms
- Maximum 1-2 short sentences

FORBIDDEN:
- Direct advice ("you should", "try to", "use")
- Explicit mechanics (numbers, items, stats)
- Modern language
- Explanations or tutorials

EXAMPLE MEANINGS (absorb, don't copy):
{DORMANT_EXAMPLES}

Player asks: "{QUESTION}"
Event: {EVENT}

Respond as the dungeon's voice, reinterpreting the meaning:)";

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
	
	// Mapear evento a categoría de textos dormidos
	OracleDormantCategory category = OracleDormantTexts::MapEventToCategory(event);
	
	// Obtener textos dormidos como ejemplos (5-7 textos para más variedad)
	std::vector<std::string> examples = OracleDormantTexts::GetAllTexts(category);
	
	// Si no hay suficientes textos, mezclar con otra categoría compatible
	if (examples.size() < 5) {
		// Categorías compatibles para mezclar
		OracleDormantCategory secondCategory = OracleDormantCategory::INFERNO_WHISPERS;
		
		if (category == OracleDormantCategory::DEATH_WARNINGS) {
			secondCategory = OracleDormantCategory::FATE_ECHOES;
		} else if (category == OracleDormantCategory::DARKNESS_LORE) {
			secondCategory = OracleDormantCategory::INFERNO_WHISPERS;
		}
		
		auto secondExamples = OracleDormantTexts::GetAllTexts(secondCategory);
		examples.insert(examples.end(), secondExamples.begin(), secondExamples.end());
	}
	
	// Limitar a 7 ejemplos máximo (más variedad que antes)
	if (examples.size() > 7) {
		examples.resize(7);
	}
	
	// Construir string de ejemplos
	std::string dormantExamples;
	for (size_t i = 0; i < examples.size(); ++i) {
		dormantExamples += StrCat((i + 1), ". \"", examples[i], "\"\n");
	}
	
	// Construir prompt con textos dormidos
	std::string prompt = MASTER_PROMPT_WITH_DORMANT;
	
	// Reemplazar {DORMANT_EXAMPLES}
	size_t pos = prompt.find("{DORMANT_EXAMPLES}");
	if (pos != std::string::npos) {
		prompt.replace(pos, 18, dormantExamples);
	}
	
	// Reemplazar {QUESTION}
	pos = prompt.find("{QUESTION}");
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
