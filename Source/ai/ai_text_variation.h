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
    
    // 🪙 TOKEN BUCKET: Rate limiting mejorado
    int tokensPerDay = 100;           // Tokens disponibles por día
    int costPerCall = 1;              // Costo por llamada
    int minSecondsBetweenCalls = 5;   // Cooldown corto entre llamadas
    
    // 🔍 TELEMETRY: Modo silencioso para testing
    bool silentMode = false;          // Procesa pero no muestra variaciones
    
    // 📏 LORE-SAFE: Validación mejorada
    float maxLengthMultiplier = 1.2f; // Máximo 20% más largo que original
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
// 🪙 TOKEN BUCKET SYSTEM
// ============================================================================

/**
 * Obtiene tokens restantes hoy
 */
int GetRemainingTokens();

/**
 * Resetea tokens (llamar al inicio del día)
 */
void ResetDailyTokens();

/**
 * Verifica si hay tokens disponibles
 */
bool HasTokensAvailable();

// ============================================================================
// 📊 TELEMETRY SYSTEM
// ============================================================================

/**
 * Registra un evento de IA (invisible para jugador)
 * 
 * @param npc NPC o contexto
 * @param context Estado del mundo (día/noche, etc.)
 * @param success Si la llamada fue exitosa
 */
void LogAIEvent(const std::string& npc, const std::string& context, bool success);

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
    uint32_t lengthRejections = 0;      // 📏 Rechazos por longitud excesiva
    uint32_t averageLatencyMs = 0;
    uint32_t tokenBucketRejections = 0; // 🪙 Rechazos por falta de tokens
    uint32_t tokensRemaining = 0;       // 🪙 Tokens restantes hoy
    
    // 📊 TELEMETRY: Momentos IA (invisible para jugador)
    struct AIEvent {
        std::string npc;
        std::string context;
        uint32_t timestamp;
        bool success;
    };
    // Últimos 10 eventos (para análisis)
    std::vector<AIEvent> recentEvents;
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
