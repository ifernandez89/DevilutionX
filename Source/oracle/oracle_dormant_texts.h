/**
 * @file oracle_dormant_texts.h
 * 
 * 🔮 EL ORÁCULO - Sistema de Textos Dormidos
 * 
 * FILOSOFÍA CRÍTICA:
 * ==================
 * La IA NO crea contenido libre. La IA SOLO hace micro-variaciones de textos existentes.
 * 
 * PIPELINE:
 * 1. Texto base original/dormido (Blizzard North 1996) → 70%
 * 2. IA analiza y hace ligera variación → 30%
 * 3. Validación lore-safe → 100%
 * 
 * REGLAS ESTRICTAS:
 * ❌ NO creatividad libre
 * ❌ NO modernismos
 * ❌ NO explicaciones
 * ✅ SOLO reordenar material existente
 * ✅ SOLO micro-variaciones validadas
 * ✅ SOLO textos originales como base
 * 
 * Esto es arqueología digital asistida, NO generación creativa.
 */

#pragma once

#include <string>
#include <vector>
#include <optional>

namespace devilution {

/**
 * @brief Categorías de textos dormidos del Oráculo
 */
enum class OracleDormantCategory {
	DEATH_WARNINGS,      // Advertencias sobre muerte
	DARKNESS_LORE,       // Lore sobre oscuridad
	ANCIENT_WISDOM,      // Sabiduría antigua
	CRYPTIC_PROPHECY,    // Profecías crípticas
	INFERNO_WHISPERS,    // Susurros del infierno
	FATE_ECHOES          // Ecos del destino
};

/**
 * @brief Entrada de texto dormido
 */
struct DormantTextEntry {
	const char* text;
	OracleDormantCategory category;
	const char* context;     // Cuándo usar este texto
	int priority;            // 1-10 (10 = más relevante)
};

/**
 * @brief Sistema de Textos Dormidos del Oráculo
 * 
 * Gestiona la base de datos de textos dormidos auténticos de Diablo
 * y los proporciona al Oráculo para respuestas lore-safe.
 */
class OracleDormantTexts {
public:
	/**
	 * @brief Obtiene un texto dormido aleatorio por categoría
	 * 
	 * @param category Categoría de texto deseada
	 * @return Texto dormido o nullopt si no hay disponible
	 */
	static std::optional<std::string> GetRandomText(OracleDormantCategory category);
	
	/**
	 * @brief Obtiene un texto dormido contextual
	 * 
	 * Selecciona el texto más apropiado según el evento y estado del jugador.
	 * 
	 * @param event Evento que disparó la respuesta
	 * @param playerState Estado del jugador (FRIENDLY/ATTACK)
	 * @return Texto dormido contextual
	 */
	static std::optional<std::string> GetContextualText(
		const std::string& event,
		const std::string& playerState
	);
	
	/**
	 * @brief Obtiene todos los textos de una categoría
	 * 
	 * @param category Categoría de textos
	 * @return Vector de textos dormidos
	 */
	static std::vector<std::string> GetAllTexts(OracleDormantCategory category);
	
	/**
	 * @brief Construye un prompt que incluye textos dormidos como ejemplos
	 * 
	 * Esto ayuda a la IA a generar respuestas en el mismo estilo.
	 * 
	 * @param question Pregunta del jugador
	 * @param category Categoría de textos a usar como ejemplos
	 * @return Prompt enriquecido con ejemplos de textos dormidos
	 */
	static std::string BuildPromptWithDormantExamples(
		const std::string& question,
		OracleDormantCategory category
	);
	
	/**
	 * @brief Verifica si hay textos dormidos disponibles
	 * @return true si hay textos en la base de datos
	 */
	static bool HasDormantTexts();
	
	/**
	 * @brief Obtiene el número total de textos dormidos
	 * @return Cantidad de textos en la base de datos
	 */
	static size_t GetTextCount();
	
	/**
	 * @brief Mapea evento a categoría de texto
	 * @param event Nombre del evento
	 * @return Categoría más apropiada
	 */
	static OracleDormantCategory MapEventToCategory(const std::string& event);
	
private:
	
	/**
	 * @brief Selecciona textos de ejemplo para el prompt
	 * @param category Categoría de textos
	 * @param count Número de ejemplos a incluir
	 * @return Vector de textos de ejemplo
	 */
	static std::vector<std::string> SelectExampleTexts(
		OracleDormantCategory category,
		int count = 3
	);
};

} // namespace devilution
