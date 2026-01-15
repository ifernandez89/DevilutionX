/**
 * @file ai_text_variation.h
 * 
 * 🤖 AI TEXT VARIATION SYSTEM 🤖
 * 
 * Sistema opcional de variación de texto usando IA para añadir dinamismo
 * a diálogos y mensajes sin inventar lore nuevo.
 * 
 * PRINCIPIOS:
 * - Fallback garantizado: Si falla, usa texto original
 * - No invasivo: El juego funciona 100% sin IA
 * - Lore-safe: Solo usa palabras que ya existen en el texto base
 * - Opcional: Se activa con API key en config
 */

#pragma once

#include <string>
#include <optional>
#include <cstdint>

namespace devilution {

// ============================================================================
// 🔧 CONFIGURACIÓN
// ============================================================================

/**
 * Configuración del sistema de IA
 */
struct AIConfig {
    std::string apiKey;
    std::string baseURL = "https://openrouter.ai/api/v1";
    std::string model = "mistralai/mistral-7b-instruct:free";
    int timeoutMs = 400;  // 🔥 TIMEOUT AGRESIVO: 400ms máximo
    int maxTokens = 128;
    float temperature = 0.6f;
    bool enabled = false;
    
    // 🛡️ RATE LIMITING: Budget controlled
    int minSecondsBetweenCalls = 15;  // 1 request cada 15 segundos GLOBAL
};

/**
 * Tonos disponibles para variación de texto
 */
enum class AITone {
    Neutral,    // Tono por defecto
    Weary,      // Cansado, exhausto
    Cold,       // Frío, distante
    Distant,    // Lejano, desinteresado
    Cryptic,    // Críptico, misterioso
    Irritated   // Irritado, molesto
};

// ============================================================================
// 🎯 API PRINCIPAL
// ============================================================================

/**
 * Inicializa el sistema de IA
 * Lee configuración de nightmare_config/ai.ini
 */
void InitAITextVariation();

/**
 * Limpia el sistema de IA
 */
void CleanupAITextVariation();

/**
 * Intenta variar texto con IA
 * 
 * @param text Texto original a variar
 * @param tone Tono deseado para la variación
 * @return Texto variado si tiene éxito, nullopt si falla
 * 
 * GARANTÍAS:
 * - Nunca crashea
 * - Nunca bloquea más de timeoutMs
 * - Retorna nullopt en cualquier error
 */
std::optional<std::string> TryAITextVariation(
    const std::string& text,
    AITone tone = AITone::Neutral
);

/**
 * Valida que la respuesta IA solo use palabras del texto base
 * 
 * @param aiText Texto generado por IA
 * @param baseText Texto original
 * @return true si es seguro (no inventa palabras nuevas)
 */
bool IsLoreSafe(const std::string& aiText, const std::string& baseText);

/**
 * Procesa un mensaje de chat con IA (si está disponible)
 * 
 * @param input Mensaje del jugador
 * @return Mensaje variado o original si IA no disponible
 */
std::string ProcessChatMessageWithAI(const std::string& input);

// ============================================================================
// 🔧 CONFIGURACIÓN Y ESTADO
// ============================================================================

/**
 * Obtiene la configuración actual
 */
const AIConfig& GetAIConfig();

/**
 * Establece si el sistema está habilitado
 */
void SetAIEnabled(bool enabled);

/**
 * Verifica si el sistema está habilitado y configurado
 */
bool IsAIAvailable();

/**
 * Limpia el cache de respuestas
 */
void ClearAICache();

// ============================================================================
// 📊 ESTADÍSTICAS
// ============================================================================

/**
 * Estadísticas del sistema de IA
 */
struct AIStats {
    uint32_t totalRequests = 0;
    uint32_t successfulRequests = 0;
    uint32_t failedRequests = 0;
    uint32_t cachedResponses = 0;
    uint32_t loreSafeRejections = 0;
    uint32_t averageLatencyMs = 0;
};

/**
 * Obtiene estadísticas del sistema
 */
AIStats GetAIStats();

/**
 * Resetea estadísticas
 */
void ResetAIStats();

// ============================================================================
// 🔍 DEBUGGING
// ============================================================================

#ifdef _DEBUG
/**
 * Imprime estadísticas del sistema de IA
 */
void DebugPrintAIStats();

/**
 * Habilita/deshabilita logging detallado
 */
void SetAIDebugLogging(bool enabled);

/**
 * Fuerza un test de conectividad
 */
bool TestAIConnection();
#endif

} // namespace devilution
