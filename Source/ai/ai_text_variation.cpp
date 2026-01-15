/**
 * @file ai_text_variation.cpp
 * 
 * 🤖 AI TEXT VARIATION SYSTEM 🤖
 * 
 * Implementación del sistema de variación de texto con IA
 */

#include "ai_text_variation.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

#include "utils/log.hpp"
#include "utils/str_cat.hpp"

// Nota: La implementación HTTP real requeriría libcurl o similar
// Por ahora, implementamos la estructura y validación

namespace devilution {

// ============================================================================
// 🔒 ESTADO GLOBAL
// ============================================================================

namespace {

AIConfig g_aiConfig;
AIStats g_aiStats;

// Cache de respuestas (key = text + "|" + tone)
std::unordered_map<std::string, std::string> g_aiCache;
constexpr size_t MAX_CACHE_SIZE = 100;

// 🛡️ RATE LIMITING: Global cooldown para IA
uint32_t g_lastAICallTime = 0;

#ifdef _DEBUG
bool g_debugLogging = false;
#endif

/**
 * Convierte texto a minúsculas
 */
std::string ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return result;
}

/**
 * Convierte AITone a string para el prompt
 */
std::string ToneToString(AITone tone) {
    switch (tone) {
        case AITone::Neutral: return "neutral";
        case AITone::Weary: return "weary";
        case AITone::Cold: return "cold";
        case AITone::Distant: return "distant";
        case AITone::Cryptic: return "cryptic";
        case AITone::Irritated: return "irritated";
        default: return "neutral";
    }
}

/**
 * 🛡️ BUDGET CONTROLLED: Verifica si podemos llamar a la IA
 * Regla simple: 1 request cada X segundos GLOBAL (no por NPC)
 */
bool CanCallAI() {
    uint32_t currentTime = SDL_GetTicks();
    uint32_t minInterval = g_aiConfig.minSecondsBetweenCalls * 1000;
    
    if (currentTime - g_lastAICallTime < minInterval) {
#ifdef _DEBUG
        if (g_debugLogging) {
            uint32_t remaining = (minInterval - (currentTime - g_lastAICallTime)) / 1000;
            LogVerbose("AI: Rate limited, wait {}s", remaining);
        }
#endif
        return false;
    }
    
    return true;
}

/**
 * Construye el prompt para la IA
 */
std::string BuildPrompt(const std::string& text, AITone tone) {
    return StrCat(
        "You are a text variation engine for Diablo 1 (1996).\n\n"
        "GOAL: Create NOTICEABLE variations that feel alive and dynamic while respecting lore.\n\n"
        "STRICT RULES:\n"
        "- You MUST NOT invent new lore, facts, places, names, events, or mechanics.\n"
        "- You MUST NOT add new information.\n"
        "- You MUST ONLY use words from the ORIGINAL TEXT (you can add common words like: the, a, is, are, but, yet, still, now, here, there).\n"
        "- You SHOULD create NOTICEABLE variations: reorder words dramatically, change rhythm, add pauses (...), change emphasis.\n"
        "- You MAY use ellipsis (...), capitalization for EMPHASIS, question marks, exclamation points.\n"
        "- Make it feel ALIVE and REACTIVE, not robotic.\n"
        "- Output ONLY the final text, no explanations.\n\n"
        "STYLE:\n"
        "- Dark, gothic, medieval\n"
        "- Diablo 1 tone (1996)\n"
        "- Dramatic and atmospheric\n"
        "- No modern slang\n"
        "- No humor\n\n"
        "ORIGINAL TEXT: \"", text, "\"\n\n"
        "TONE MODIFIER: ", ToneToString(tone), "\n\n"
        "Examples of good variations:\n"
        "- \"What can I do for you?\" → \"What... can I do for you?\" (weary)\n"
        "- \"The darkness grows.\" → \"The darkness... it GROWS.\" (cryptic)\n"
        "- \"Stay awhile and listen.\" → \"Stay. Listen awhile.\" (cold)"
    );
}

/**
 * Simula una llamada HTTP (stub para testing)
 * En producción, esto usaría libcurl o similar
 */
std::optional<std::string> CallOpenRouter(
    const std::string& prompt,
    const AIConfig& config
) {
    // 🚧 STUB: En producción, esto haría una llamada HTTP real
    // Por ahora, retornamos nullopt para simular que no está disponible
    
#ifdef _DEBUG
    if (g_debugLogging) {
        LogVerbose("AI: Would call OpenRouter with prompt length: {}", prompt.length());
    }
#endif
    
    // TODO: Implementar llamada HTTP real con libcurl
    // - POST a config.baseURL + "/chat/completions"
    // - Headers: Authorization: Bearer {config.apiKey}
    // - Body: JSON con model, messages, temperature, max_tokens
    // - Timeout: config.timeoutMs
    
    return std::nullopt;
}

} // namespace

// ============================================================================
// 🎯 IMPLEMENTACIÓN DE LA API PRINCIPAL
// ============================================================================

void InitAITextVariation()
{
    // Configuración por defecto
    g_aiConfig = AIConfig{};
    g_aiStats = AIStats{};
    g_aiCache.clear();
    
    // TODO: Leer configuración de nightmare_config/ai.ini
    // Por ahora, el sistema está deshabilitado por defecto
    g_aiConfig.enabled = false;
    
#ifdef _DEBUG
    LogVerbose("AI Text Variation System initialized (disabled by default)");
#endif
}

void CleanupAITextVariation()
{
    g_aiCache.clear();
    g_aiConfig = AIConfig{};
    
#ifdef _DEBUG
    LogVerbose("AI Text Variation System cleaned up");
#endif
}

std::optional<std::string> TryAITextVariation(
    const std::string& text,
    AITone tone
)
{
    g_aiStats.totalRequests++;
    
    // Check 1: Sistema habilitado?
    if (!g_aiConfig.enabled || g_aiConfig.apiKey.empty()) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: System disabled or no API key");
        }
#endif
        g_aiStats.failedRequests++;
        return std::nullopt;
    }
    
    // Check 2: Texto válido?
    if (text.empty() || text.length() > 500) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: Invalid text length: {}", text.length());
        }
#endif
        g_aiStats.failedRequests++;
        return std::nullopt;
    }
    
    // Check 3: Cache hit?
    std::string cacheKey = text + "|" + ToneToString(tone);
    auto cacheIt = g_aiCache.find(cacheKey);
    if (cacheIt != g_aiCache.end()) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: Cache hit for: {}", text);
        }
#endif
        g_aiStats.cachedResponses++;
        return cacheIt->second;
    }
    
    // 🛡️ Check 4: RATE LIMITING - Budget controlled
    // 1 request cada X segundos GLOBAL (no por NPC)
    if (!CanCallAI()) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: Rate limited, using original text");
        }
#endif
        g_aiStats.failedRequests++;
        return std::nullopt;  // Fallback a texto original
    }
    
    // Llamar IA
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::string prompt = BuildPrompt(text, tone);
    auto aiResult = CallOpenRouter(prompt, g_aiConfig);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // 🛡️ REGISTRAR TIEMPO DE LLAMADA (para rate limiting)
    g_lastAICallTime = SDL_GetTicks();
    
    // Actualizar latencia promedio
    g_aiStats.averageLatencyMs = 
        (g_aiStats.averageLatencyMs * g_aiStats.successfulRequests + duration.count()) /
        (g_aiStats.successfulRequests + 1);
    
    if (!aiResult.has_value()) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: Request failed");
        }
#endif
        g_aiStats.failedRequests++;
        return std::nullopt;
    }
    
    // Validar lore safety
    if (!IsLoreSafe(*aiResult, text)) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: Lore safety check failed for: {}", *aiResult);
        }
#endif
        g_aiStats.loreSafeRejections++;
        g_aiStats.failedRequests++;
        return std::nullopt;
    }
    
    // Éxito: guardar en cache
    g_aiStats.successfulRequests++;
    
    if (g_aiCache.size() >= MAX_CACHE_SIZE) {
        g_aiCache.clear(); // Simple cache eviction
    }
    g_aiCache[cacheKey] = *aiResult;
    
#ifdef _DEBUG
    if (g_debugLogging) {
        LogVerbose("AI: Success - {} -> {}", text, *aiResult);
    }
#endif
    
    return *aiResult;
}

bool IsLoreSafe(const std::string& aiText, const std::string& baseText)
{
    // Palabras comunes permitidas (artículos, preposiciones, etc.)
    static const std::unordered_set<std::string> commonWords = {
        "a", "an", "the", "is", "are", "was", "were", "be", "been", "being",
        "have", "has", "had", "do", "does", "did", "will", "would", "could",
        "should", "may", "might", "must", "can", "of", "to", "in", "for",
        "on", "at", "by", "with", "from", "as", "into", "through", "during",
        "before", "after", "above", "below", "between", "under", "again",
        "further", "then", "once", "here", "there", "when", "where", "why",
        "how", "all", "both", "each", "few", "more", "most", "other", "some",
        "such", "no", "nor", "not", "only", "own", "same", "so", "than",
        "too", "very", "s", "t", "d", "ll", "ve", "re", "m"
    };
    
    // Extraer palabras del texto base
    std::unordered_set<std::string> baseWords;
    std::istringstream baseStream(ToLower(baseText));
    std::string word;
    
    while (baseStream >> word) {
        // Limpiar puntuación
        word.erase(std::remove_if(word.begin(), word.end(),
            [](unsigned char c) { return !std::isalnum(c); }), word.end());
        if (!word.empty()) {
            baseWords.insert(word);
        }
    }
    
    // Verificar que todas las palabras IA existen en base o son comunes
    std::istringstream aiStream(ToLower(aiText));
    while (aiStream >> word) {
        word.erase(std::remove_if(word.begin(), word.end(),
            [](unsigned char c) { return !std::isalnum(c); }), word.end());
        if (!word.empty() && 
            baseWords.find(word) == baseWords.end() &&
            commonWords.find(word) == commonWords.end()) {
#ifdef _DEBUG
            if (g_debugLogging) {
                LogVerbose("AI: Lore safety violation - new word: {}", word);
            }
#endif
            return false; // Palabra nueva detectada
        }
    }
    
    return true;
}

std::string ProcessChatMessageWithAI(const std::string& input)
{
    // Intentar IA primero
    auto aiResult = TryAITextVariation(input, AITone::Neutral);
    
    if (aiResult.has_value()) {
        return *aiResult;
    }
    
    // Fallback: texto original
    return input;
}

// ============================================================================
// 🔧 CONFIGURACIÓN Y ESTADO
// ============================================================================

const AIConfig& GetAIConfig()
{
    return g_aiConfig;
}

void SetAIEnabled(bool enabled)
{
    g_aiConfig.enabled = enabled;
    
#ifdef _DEBUG
    LogVerbose("AI Text Variation {}", enabled ? "enabled" : "disabled");
#endif
}

bool IsAIAvailable()
{
    return g_aiConfig.enabled && !g_aiConfig.apiKey.empty();
}

void ClearAICache()
{
    g_aiCache.clear();
    
#ifdef _DEBUG
    LogVerbose("AI cache cleared");
#endif
}

// ============================================================================
// 📊 ESTADÍSTICAS
// ============================================================================

AIStats GetAIStats()
{
    return g_aiStats;
}

void ResetAIStats()
{
    g_aiStats = AIStats{};
    
#ifdef _DEBUG
    LogVerbose("AI stats reset");
#endif
}

// ============================================================================
// 🔍 DEBUGGING
// ============================================================================

#ifdef _DEBUG
void DebugPrintAIStats()
{
    std::cout << "\n=== AI TEXT VARIATION STATISTICS ===" << std::endl;
    std::cout << "System enabled: " << (g_aiConfig.enabled ? "YES" : "NO") << std::endl;
    std::cout << "API key configured: " << (!g_aiConfig.apiKey.empty() ? "YES" : "NO") << std::endl;
    
    std::cout << "\n--- REQUESTS ---" << std::endl;
    std::cout << "Total requests: " << g_aiStats.totalRequests << std::endl;
    std::cout << "Successful: " << g_aiStats.successfulRequests << std::endl;
    std::cout << "Failed: " << g_aiStats.failedRequests << std::endl;
    std::cout << "Cached responses: " << g_aiStats.cachedResponses << std::endl;
    
    std::cout << "\n--- VALIDATION ---" << std::endl;
    std::cout << "Lore-safe rejections: " << g_aiStats.loreSafeRejections << std::endl;
    
    std::cout << "\n--- PERFORMANCE ---" << std::endl;
    std::cout << "Average latency: " << g_aiStats.averageLatencyMs << "ms" << std::endl;
    std::cout << "Cache size: " << g_aiCache.size() << "/" << MAX_CACHE_SIZE << std::endl;
    
    std::cout << "====================================" << std::endl;
}

void SetAIDebugLogging(bool enabled)
{
    g_debugLogging = enabled;
    std::cout << "AI debug logging " << (enabled ? "enabled" : "disabled") << std::endl;
}

bool TestAIConnection()
{
    std::cout << "Testing AI connection..." << std::endl;
    
    if (g_aiConfig.apiKey.empty()) {
        std::cout << "❌ No API key configured" << std::endl;
        return false;
    }
    
    // Test simple
    auto result = TryAITextVariation("Greetings, stranger.", AITone::Neutral);
    
    if (result.has_value()) {
        std::cout << "✅ AI connection successful" << std::endl;
        std::cout << "Response: " << *result << std::endl;
        return true;
    } else {
        std::cout << "❌ AI connection failed" << std::endl;
        return false;
    }
}
#endif

} // namespace devilution
