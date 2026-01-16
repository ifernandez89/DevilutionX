/**
 * @file oracle_validator.cpp
 * 
 * 🔮 EL ORÁCULO - Implementación del Sistema de Validación
 */

#include "oracle_validator.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <unordered_set>

#include "utils/log.hpp"

namespace devilution {

namespace {

// Palabras prohibidas (modernismos, términos técnicos, etc.)
const std::unordered_set<std::string> g_bannedWords = {
	// Modernismos
	"lol", "lmao", "omg", "wtf", "noob", "pro", "epic", "legendary",
	"awesome", "cool", "dude", "bro", "yeah", "nope", "okay",
	
	// Términos técnicos/meta
	"tutorial", "guide", "walkthrough", "strategy", "tips", "tricks",
	"bug", "glitch", "exploit", "cheat", "hack", "mod",
	"developer", "programmer", "code", "script", "ai", "algorithm",
	"player", "character", "npc", "quest", "level", "dungeon",
	
	// Términos demasiado modernos
	"internet", "computer", "digital", "virtual", "online", "offline",
	"download", "upload", "server", "client", "database"
};

// Palabras clave del tono dark/cryptic (deben aparecer algunas)
const std::unordered_set<std::string> g_toneKeywords = {
	"darkness", "dark", "shadow", "shadows", "death", "dead", "die", "dying",
	"fallen", "fall", "doom", "doomed", "curse", "cursed", "evil", "malevolent",
	"demon", "demons", "hell", "inferno", "abyss", "void", "eternal", "ancient",
	"whisper", "whispers", "echo", "echoes", "fate", "destiny", "soul", "souls",
	"mortal", "mortals", "blood", "suffer", "suffering", "pain", "agony",
	"corruption", "corrupt", "taint", "tainted", "horror", "terror", "fear"
};

} // namespace

// ============================================================================
// 🔮 IMPLEMENTACIÓN
// ============================================================================

ValidationResult OracleValidator::ValidateResponse(
	const std::string& response,
	const std::vector<std::string>& baseTexts
)
{
	ValidationResult result;
	result.isValid = false;
	result.similarity = 0.0f;
	result.reason = "";
	result.baseTextUsed = "";
	
	// 1. Verificar que la respuesta no esté vacía
	if (response.empty() || response.length() < 10) {
		result.reason = "Response too short";
		return result;
	}
	
	// 2. Verificar palabras prohibidas
	if (ContainsBannedWords(response)) {
		result.reason = "Contains banned words (modernisms or technical terms)";
		return result;
	}
	
	// 3. Verificar tono consistente
	if (!HasConsistentTone(response)) {
		result.reason = "Tone not consistent with dark/cryptic style";
		return result;
	}
	
	// 4. Calcular similitud con cada texto base
	float maxSimilarity = 0.0f;
	std::string bestMatch;
	
	for (const auto& baseText : baseTexts) {
		float similarity = CalculateSimilarity(response, baseText);
		
		if (similarity > maxSimilarity) {
			maxSimilarity = similarity;
			bestMatch = baseText;
		}
	}
	
	result.similarity = maxSimilarity;
	result.baseTextUsed = bestMatch;
	
	// 5. Verificar similitud mínima (60%)
	if (maxSimilarity < 0.6f) {
		result.reason = "Similarity too low - not a recognizable variation";
		return result;
	}
	
	// 6. Verificar longitud apropiada
	if (!bestMatch.empty() && !IsLengthAppropriate(response, bestMatch)) {
		result.reason = "Length differs too much from base text";
		return result;
	}
	
	// 7. Todo OK
	result.isValid = true;
	result.reason = "Valid micro-variation";
	
#ifdef _DEBUG
	LogVerbose("Oracle Validator: Response valid (similarity: {:.2f})", maxSimilarity);
#endif
	
	return result;
}

float OracleValidator::CalculateSimilarity(
	const std::string& text1,
	const std::string& text2
)
{
	// Extraer palabras de ambos textos
	std::vector<std::string> words1 = ExtractWords(text1);
	std::vector<std::string> words2 = ExtractWords(text2);
	
	if (words1.empty() || words2.empty()) {
		return 0.0f;
	}
	
	// Contar palabras compartidas
	std::unordered_set<std::string> set1(words1.begin(), words1.end());
	std::unordered_set<std::string> set2(words2.begin(), words2.end());
	
	int sharedWords = 0;
	for (const auto& word : set1) {
		if (set2.find(word) != set2.end()) {
			sharedWords++;
		}
	}
	
	// Calcular similitud (Jaccard similarity)
	int totalUniqueWords = set1.size() + set2.size() - sharedWords;
	
	if (totalUniqueWords == 0) {
		return 0.0f;
	}
	
	return static_cast<float>(sharedWords) / static_cast<float>(totalUniqueWords);
}

bool OracleValidator::ContainsBannedWords(const std::string& text)
{
	std::string normalized = NormalizeText(text);
	std::vector<std::string> words = ExtractWords(normalized);
	
	for (const auto& word : words) {
		if (g_bannedWords.find(word) != g_bannedWords.end()) {
#ifdef _DEBUG
			LogVerbose("Oracle Validator: Banned word detected: '{}'", word);
#endif
			return true;
		}
	}
	
	return false;
}

bool OracleValidator::IsLengthAppropriate(
	const std::string& response,
	const std::string& baseText
)
{
	size_t responseLen = response.length();
	size_t baseLen = baseText.length();
	
	if (baseLen == 0) {
		return true;
	}
	
	// Permitir ±30% de diferencia
	float ratio = static_cast<float>(responseLen) / static_cast<float>(baseLen);
	
	return (ratio >= 0.7f && ratio <= 1.3f);
}

bool OracleValidator::HasConsistentTone(const std::string& text)
{
	std::vector<std::string> words = ExtractWords(NormalizeText(text));
	
	// Contar palabras clave del tono
	int toneWordCount = 0;
	
	for (const auto& word : words) {
		if (g_toneKeywords.find(word) != g_toneKeywords.end()) {
			toneWordCount++;
		}
	}
	
	// Debe tener al menos 2 palabras clave del tono dark/cryptic
	// o al menos 10% de las palabras deben ser palabras clave
	int minToneWords = std::max(2, static_cast<int>(words.size()) / 10);
	
	return toneWordCount >= minToneWords;
}

std::vector<std::string> OracleValidator::ExtractWords(const std::string& text)
{
	std::vector<std::string> words;
	std::string normalized = NormalizeText(text);
	std::istringstream stream(normalized);
	std::string word;
	
	while (stream >> word) {
		if (word.length() >= 3) {  // Ignorar palabras muy cortas
			words.push_back(word);
		}
	}
	
	return words;
}

std::string OracleValidator::NormalizeText(const std::string& text)
{
	std::string normalized;
	normalized.reserve(text.length());
	
	for (char c : text) {
		if (std::isalpha(static_cast<unsigned char>(c))) {
			normalized += std::tolower(static_cast<unsigned char>(c));
		} else if (std::isspace(static_cast<unsigned char>(c))) {
			normalized += ' ';
		}
	}
	
	return normalized;
}

} // namespace devilution
