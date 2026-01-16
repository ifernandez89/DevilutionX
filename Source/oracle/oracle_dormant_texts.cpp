/**
 * @file oracle_dormant_texts.cpp
 * 
 * 🔮 EL ORÁCULO - Implementación de Textos Dormidos
 * 
 * Base de datos de textos originales y dormidos de Diablo.
 * La IA usa estos textos como BASE OBLIGATORIA para micro-variaciones.
 */

#include "oracle_dormant_texts.h"

#include <algorithm>
#include <SDL.h>

#include "utils/str_cat.hpp"

namespace devilution {

// ============================================================================
// 🔮 BASE DE DATOS DE TEXTOS DORMIDOS
// ============================================================================

/**
 * IMPORTANTE: Estos textos son ejemplos del ESTILO de textos dormidos.
 * En producción, se extraerían de los archivos originales del juego.
 * 
 * Categorías basadas en diálogos existentes de Diablo 1:
 * - Cain, Adria, Griswold, Pepin, Farnham
 * - Libros de lore
 * - Textos de quest
 */

const DormantTextEntry g_dormantTextsDatabase[] = {
	// ========================================================================
	// DEATH_WARNINGS - Advertencias sobre muerte y peligro
	// ========================================================================
	{
		"The darkness grows stronger with each passing moment. "
		"Many have fallen before you, and many more shall follow.",
		OracleDormantCategory::DEATH_WARNINGS,
		"Muerte del jugador, niveles profundos",
		9
	},
	{
		"These halls have claimed countless souls. "
		"The stones themselves remember every scream.",
		OracleDormantCategory::DEATH_WARNINGS,
		"Muerte en cathedral/dungeon",
		8
	},
	{
		"Death is but a doorway in these cursed depths. "
		"Each fall brings you closer to understanding the true horror.",
		OracleDormantCategory::DEATH_WARNINGS,
		"Muerte repetida",
		7
	},
	
	// ========================================================================
	// DARKNESS_LORE - Lore sobre oscuridad y corrupción
	// ========================================================================
	{
		"The corruption spreads like a plague through these ancient stones. "
		"What was once holy now festers with malevolent power.",
		OracleDormantCategory::DARKNESS_LORE,
		"Cathedral levels, lore general",
		9
	},
	{
		"Long ago, these halls echoed with prayers and hymns. "
		"Now only the whispers of the damned remain.",
		OracleDormantCategory::DARKNESS_LORE,
		"Entrada a cathedral, contexto histórico",
		8
	},
	{
		"The very air grows thick with evil. "
		"Even the bravest warriors feel the weight of this darkness.",
		OracleDormantCategory::DARKNESS_LORE,
		"Niveles profundos, atmósfera",
		7
	},
	
	// ========================================================================
	// ANCIENT_WISDOM - Sabiduría antigua y consejos crípticos
	// ========================================================================
	{
		"The wise know when to retreat and when to press forward. "
		"Recklessness serves only to feed the darkness.",
		OracleDormantCategory::ANCIENT_WISDOM,
		"Jugador muriendo frecuentemente",
		8
	},
	{
		"Preparation is the key to survival in these depths. "
		"Those who rush headlong meet only swift demise.",
		OracleDormantCategory::ANCIENT_WISDOM,
		"Consejo sobre estrategia",
		7
	},
	{
		"The old texts speak of patience and cunning. "
		"Brute force alone will not prevail against such evil.",
		OracleDormantCategory::ANCIENT_WISDOM,
		"Consejo táctico",
		6
	},
	
	// ========================================================================
	// CRYPTIC_PROPHECY - Profecías y presagios
	// ========================================================================
	{
		"The signs are clear to those who know how to read them. "
		"Your path leads ever downward into shadow.",
		OracleDormantCategory::CRYPTIC_PROPHECY,
		"Progreso del jugador",
		8
	},
	{
		"It is written that a hero shall descend into darkness. "
		"Whether they emerge victorious or join the fallen remains to be seen.",
		OracleDormantCategory::CRYPTIC_PROPHECY,
		"Profecía general",
		9
	},
	{
		"The stars themselves have turned away from this place. "
		"Fate hangs by the thinnest of threads.",
		OracleDormantCategory::CRYPTIC_PROPHECY,
		"Presagio oscuro",
		7
	},
	
	// ========================================================================
	// INFERNO_WHISPERS - Susurros del infierno y voces oscuras
	// ========================================================================
	{
		"Can you hear them? The whispers that echo through these corridors? "
		"They speak of things best left forgotten.",
		OracleDormantCategory::INFERNO_WHISPERS,
		"Atmósfera de horror",
		8
	},
	{
		"The walls themselves seem to breathe with malice. "
		"This place is alive with ancient hatred.",
		OracleDormantCategory::INFERNO_WHISPERS,
		"Hell levels, atmósfera",
		9
	},
	{
		"Listen carefully and you may hear the laughter of demons. "
		"They delight in the suffering of mortals.",
		OracleDormantCategory::INFERNO_WHISPERS,
		"Presencia demoníaca",
		7
	},
	
	// ========================================================================
	// FATE_ECHOES - Ecos del destino y consecuencias
	// ========================================================================
	{
		"Every choice you make echoes through eternity. "
		"The path you walk has been walked before, and will be walked again.",
		OracleDormantCategory::FATE_ECHOES,
		"Reflexión sobre destino",
		8
	},
	{
		"The cycle continues, as it always has. "
		"Heroes rise, heroes fall, and the darkness endures.",
		OracleDormantCategory::FATE_ECHOES,
		"Naturaleza cíclica",
		7
	},
	{
		"Your fate was sealed the moment you entered these cursed halls. "
		"Now you must see it through to whatever end awaits.",
		OracleDormantCategory::FATE_ECHOES,
		"Compromiso inevitable",
		9
	}
};

constexpr size_t g_dormantTextsCount = sizeof(g_dormantTextsDatabase) / sizeof(DormantTextEntry);

// ============================================================================
// 🔮 IMPLEMENTACIÓN
// ============================================================================

std::optional<std::string> OracleDormantTexts::GetRandomText(OracleDormantCategory category)
{
	// Recopilar textos de la categoría
	std::vector<const DormantTextEntry*> matchingTexts;
	
	for (size_t i = 0; i < g_dormantTextsCount; ++i) {
		if (g_dormantTextsDatabase[i].category == category) {
			matchingTexts.push_back(&g_dormantTextsDatabase[i]);
		}
	}
	
	if (matchingTexts.empty()) {
		return std::nullopt;
	}
	
	// Seleccionar uno aleatorio (ponderado por prioridad)
	int totalPriority = 0;
	for (const auto* entry : matchingTexts) {
		totalPriority += entry->priority;
	}
	
	int roll = SDL_GetTicks() % totalPriority;
	int accumulated = 0;
	
	for (const auto* entry : matchingTexts) {
		accumulated += entry->priority;
		if (roll < accumulated) {
			return std::string(entry->text);
		}
	}
	
	// Fallback: retornar el primero
	return std::string(matchingTexts[0]->text);
}

std::optional<std::string> OracleDormantTexts::GetContextualText(
	const std::string& event,
	const std::string& playerState
)
{
	// Mapear evento a categoría
	OracleDormantCategory category = MapEventToCategory(event);
	
	// Si el jugador está en peligro (ATTACK), preferir advertencias
	if (playerState == "ATTACK" || playerState == "Ominous, threatening, dark") {
		if (category != OracleDormantCategory::DEATH_WARNINGS) {
			// 60% chance de usar DEATH_WARNINGS en situaciones peligrosas
			if ((SDL_GetTicks() % 100) < 60) {
				category = OracleDormantCategory::DEATH_WARNINGS;
			}
		}
	}
	
	return GetRandomText(category);
}

std::vector<std::string> OracleDormantTexts::GetAllTexts(OracleDormantCategory category)
{
	std::vector<std::string> texts;
	
	for (size_t i = 0; i < g_dormantTextsCount; ++i) {
		if (g_dormantTextsDatabase[i].category == category) {
			texts.push_back(g_dormantTextsDatabase[i].text);
		}
	}
	
	return texts;
}

std::string OracleDormantTexts::BuildPromptWithDormantExamples(
	const std::string& question,
	OracleDormantCategory category
)
{
	// Seleccionar 3 textos de ejemplo
	std::vector<std::string> examples = SelectExampleTexts(category, 3);
	
	// Construir prompt que OBLIGA a la IA a usar estos textos como base
	std::string prompt = "You are the Oracle of the Inferno from Diablo's world.\n\n";
	
	prompt += "CRITICAL RULES:\n";
	prompt += "- You MUST base your response on the example texts below\n";
	prompt += "- You can ONLY make slight variations (reorder words, change 1-2 words)\n";
	prompt += "- You CANNOT create new content freely\n";
	prompt += "- Keep the dark, ominous tone of the examples\n";
	prompt += "- Maximum 2-3 short lines\n\n";
	
	prompt += "EXAMPLE TEXTS (use these as your BASE):\n";
	for (size_t i = 0; i < examples.size(); ++i) {
		prompt += StrCat((i + 1), ". \"", examples[i], "\"\n");
	}
	
	prompt += "\nPlayer's question: \"";
	prompt += question;
	prompt += "\"\n\n";
	
	prompt += "Respond by selecting and slightly varying ONE of the example texts above. ";
	prompt += "Your response must be recognizable as a variation of an example:";
	
	return prompt;
}

bool OracleDormantTexts::HasDormantTexts()
{
	return g_dormantTextsCount > 0;
}

size_t OracleDormantTexts::GetTextCount()
{
	return g_dormantTextsCount;
}

OracleDormantCategory OracleDormantTexts::MapEventToCategory(const std::string& event)
{
	if (event == "PLAYER_DEATH") {
		return OracleDormantCategory::DEATH_WARNINGS;
	}
	if (event == "ENTERED_TOWN") {
		return OracleDormantCategory::ANCIENT_WISDOM;
	}
	if (event == "LEVEL_CLEARED") {
		return OracleDormantCategory::FATE_ECHOES;
	}
	if (event == "BOOK_INTERACTION") {
		return OracleDormantCategory::DARKNESS_LORE;
	}
	if (event == "ALTAR_INTERACTION") {
		return OracleDormantCategory::CRYPTIC_PROPHECY;
	}
	if (event == "NPC_INTERACTION") {
		return OracleDormantCategory::ANCIENT_WISDOM;
	}
	
	// Default: usar susurros del infierno
	return OracleDormantCategory::INFERNO_WHISPERS;
}

std::vector<std::string> OracleDormantTexts::SelectExampleTexts(
	OracleDormantCategory category,
	int count
)
{
	std::vector<std::string> allTexts = GetAllTexts(category);
	std::vector<std::string> selected;
	
	if (allTexts.empty()) {
		// Fallback: usar textos de otra categoría
		allTexts = GetAllTexts(OracleDormantCategory::INFERNO_WHISPERS);
	}
	
	// Seleccionar hasta 'count' textos aleatorios
	int attempts = 0;
	while (selected.size() < static_cast<size_t>(count) && 
	       selected.size() < allTexts.size() && 
	       attempts < count * 3) {
		
		size_t index = SDL_GetTicks() % allTexts.size();
		const std::string& text = allTexts[index];
		
		// Evitar duplicados
		if (std::find(selected.begin(), selected.end(), text) == selected.end()) {
			selected.push_back(text);
		}
		
		attempts++;
	}
	
	// Si no pudimos seleccionar suficientes, añadir los primeros
	while (selected.size() < static_cast<size_t>(count) && selected.size() < allTexts.size()) {
		selected.push_back(allTexts[selected.size()]);
	}
	
	return selected;
}

} // namespace devilution
