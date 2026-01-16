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

constexpr const char* MASTER_PROMPT_WITH_DORMANT = R"(You are the Oracle of the Inferno from Diablo's dark world.

CRITICAL INSTRUCTIONS:
======================
You MUST base your response on the EXAMPLE TEXTS below.
These are authentic texts from Diablo (1996).

STRICT RULES:
- Select ONE example text as your base
- Make ONLY slight variations:
  * Reorder 1-2 phrases
  * Change 1-3 words maximum
  * Keep the same dark tone and style
- Your response must be recognizable as a variation of an example
- NO creative freedom
- NO modern language
- NO explanations or tutorials
- Maximum 2-3 short lines

EXAMPLE TEXTS (your BASE):
{DORMANT_EXAMPLES}

Context:
- Player asks: "{QUESTION}"
- Event: {EVENT}
- Mood: {TONE}

Now respond by varying ONE of the example texts above to address the player's question:)";

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
	
	// Obtener textos dormidos como ejemplos (3 textos)
	std::vector<std::string> examples = OracleDormantTexts::GetAllTexts(category);
	
	// Si no hay textos en esa categoría, usar otra
	if (examples.empty()) {
		examples = OracleDormantTexts::GetAllTexts(OracleDormantCategory::INFERNO_WHISPERS);
	}
	
	// Limitar a 3 ejemplos
	if (examples.size() > 3) {
		examples.resize(3);
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
