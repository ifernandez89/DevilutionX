/**
 * @file oracle_validator.h
 * 
 * 🔮 EL ORÁCULO - Sistema de Validación de Respuestas
 * 
 * Valida que las respuestas de la IA son micro-variaciones legítimas
 * de los textos dormidos, no contenido inventado.
 * 
 * REGLAS DE VALIDACIÓN:
 * - Respuesta debe ser similar a un texto base (>60% similitud)
 * - Longitud similar al texto base (±30%)
 * - Sin palabras prohibidas (modernismos, términos técnicos)
 * - Tono consistente (dark, cryptic, ominous)
 */

#pragma once

#include <string>
#include <optional>
#include <vector>

namespace devilution {

/**
 * @brief Resultado de validación
 */
struct ValidationResult {
	bool isValid;                    // ¿Es válida la respuesta?
	float similarity;                // Similitud con texto base (0.0-1.0)
	std::string reason;              // Razón si no es válida
	std::string baseTextUsed;        // Texto base detectado
};

/**
 * @brief Sistema de Validación del Oráculo
 * 
 * Valida que las respuestas de la IA son micro-variaciones
 * legítimas de textos dormidos.
 */
class OracleValidator {
public:
	/**
	 * @brief Valida una respuesta de la IA
	 * 
	 * @param response Respuesta generada por la IA
	 * @param baseTexts Textos base que se usaron en el prompt
	 * @return Resultado de validación
	 */
	static ValidationResult ValidateResponse(
		const std::string& response,
		const std::vector<std::string>& baseTexts
	);
	
	/**
	 * @brief Calcula similitud entre dos textos
	 * 
	 * Usa algoritmo simple de palabras compartidas.
	 * 
	 * @param text1 Primer texto
	 * @param text2 Segundo texto
	 * @return Similitud (0.0 = totalmente diferente, 1.0 = idéntico)
	 */
	static float CalculateSimilarity(
		const std::string& text1,
		const std::string& text2
	);
	
	/**
	 * @brief Verifica si el texto contiene palabras prohibidas
	 * 
	 * @param text Texto a verificar
	 * @return true si contiene palabras prohibidas
	 */
	static bool ContainsBannedWords(const std::string& text);
	
	/**
	 * @brief Verifica si la longitud es apropiada
	 * 
	 * @param response Respuesta de la IA
	 * @param baseText Texto base
	 * @return true si la longitud es similar (±30%)
	 */
	static bool IsLengthAppropriate(
		const std::string& response,
		const std::string& baseText
	);
	
	/**
	 * @brief Verifica si el tono es consistente
	 * 
	 * Busca palabras clave del tono dark/cryptic.
	 * 
	 * @param text Texto a verificar
	 * @return true si el tono es apropiado
	 */
	static bool HasConsistentTone(const std::string& text);
	
private:
	/**
	 * @brief Extrae palabras de un texto
	 * @param text Texto
	 * @return Vector de palabras (lowercase)
	 */
	static std::vector<std::string> ExtractWords(const std::string& text);
	
	/**
	 * @brief Normaliza texto (lowercase, sin puntuación)
	 * @param text Texto
	 * @return Texto normalizado
	 */
	static std::string NormalizeText(const std::string& text);
};

} // namespace devilution
