/**
 * @file ai_text_variation.cpp
 * 
 * 🤖 AI TEXT VARIATION SYSTEM 🤖
 * 
 * Implementación del sistema de variación de texto con IA
 * 
 * 🏛️ ARQUEOLOGÍA DIGITAL ASISTIDA:
 * Pipeline: Textos Dormidos (70%) → IA (25%) → Original (5%)
 */

#include "ai_text_variation.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#ifdef _DEBUG
#include <iostream>
#include <iomanip>
#endif

#include "utils/log.hpp"
#include "utils/str_cat.hpp"
#include "hidden_content.h"  // 🏛️ Integración con textos dormidos
#include "engine/random.hpp"  // Para GenerateRnd en dormant selection

// 🌐 HTTP CLIENT IMPLEMENTATION
// Platform-specific HTTP implementations
#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#else
// Para Linux/Mac, usaremos libcurl
#include <curl/curl.h>
#endif

namespace devilution {

// ============================================================================
// 🔒 ESTADO GLOBAL
// ============================================================================

namespace {

AIConfig g_aiConfig;
AIStats g_aiStats;

// Cache de respuestas (key = text + "|" + tone + "|" + context)
std::unordered_map<std::string, std::string> g_aiCache;
constexpr size_t MAX_CACHE_SIZE = 100;

// 🛡️ RATE LIMITING: Global cooldown para IA
uint32_t g_lastAICallTime = 0;

// 🪙 TOKEN BUCKET: Sistema de tokens diarios
int g_tokensRemaining = 100;
uint32_t g_lastTokenResetDay = 0;  // Día del último reset

// 🚫 SESSION FLAG: Hard-disable hasta reinicio
bool g_sessionDisabled = false;

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
 * 🌐 SIMPLE JSON BUILDER para Ollama
 * Construye JSON manualmente para el request
 */
std::string BuildJSONRequest(const std::string& model, const std::string& prompt, 
                             float temperature, int maxTokens) {
    // Escapar comillas en el prompt
    std::string escapedPrompt = prompt;
    size_t pos = 0;
    while ((pos = escapedPrompt.find('"', pos)) != std::string::npos) {
        escapedPrompt.replace(pos, 1, "\\\"");
        pos += 2;
    }
    
    // Escapar newlines
    pos = 0;
    while ((pos = escapedPrompt.find('\n', pos)) != std::string::npos) {
        escapedPrompt.replace(pos, 1, "\\n");
        pos += 2;
    }
    
    // Ollama API format con parámetros ajustados para CPU
    return StrCat(
        "{\"model\":\"", model, "\","
        "\"prompt\":\"", escapedPrompt, "\","
        "\"stream\":false,"
        "\"options\":{"
        "\"temperature\":", std::to_string(temperature), ","
        "\"top_p\":0.9,"
        "\"num_predict\":", std::to_string(maxTokens), ","
        "\"repeat_penalty\":1.05,"
        "\"stop\":[\"\\n\",\".\",\"!\",\"?\"]"
        "}}"
    );
}

/**
 * 🌐 SIMPLE JSON PARSER para Ollama
 * Extrae el contenido de la respuesta JSON
 */
std::optional<std::string> ParseJSONResponse(const std::string& json) {
    // Ollama format: {"response":"..."}
    size_t responsePos = json.find("\"response\"");
    if (responsePos == std::string::npos) {
        return std::nullopt;
    }
    
    // Buscar el inicio del valor (después de ":")
    size_t valueStart = json.find(':', responsePos);
    if (valueStart == std::string::npos) {
        return std::nullopt;
    }
    
    // Buscar la primera comilla
    valueStart = json.find('"', valueStart);
    if (valueStart == std::string::npos) {
        return std::nullopt;
    }
    valueStart++; // Saltar la comilla
    
    // Buscar la comilla de cierre (sin escapar)
    size_t valueEnd = valueStart;
    while (valueEnd < json.length()) {
        if (json[valueEnd] == '"' && (valueEnd == 0 || json[valueEnd - 1] != '\\')) {
            break;
        }
        valueEnd++;
    }
    
    if (valueEnd >= json.length()) {
        return std::nullopt;
    }
    
    std::string content = json.substr(valueStart, valueEnd - valueStart);
    
    // Desescapar caracteres básicos
    size_t pos = 0;
    while ((pos = content.find("\\n", pos)) != std::string::npos) {
        content.replace(pos, 2, "\n");
        pos++;
    }
    pos = 0;
    while ((pos = content.find("\\\"", pos)) != std::string::npos) {
        content.replace(pos, 2, "\"");
        pos++;
    }
    
    // ✂️ POST-PROCESSING OBLIGATORIO: Limpiar ruido estructural
    
    // 1. Remover "Sentence:" / "Text:" / "Output:" (case insensitive)
    std::string lower = ToLower(content);
    if (lower.find("sentence:") == 0) {
        content = content.substr(9);
    } else if (lower.find("text:") == 0) {
        content = content.substr(5);
    } else if (lower.find("output:") == 0) {
        content = content.substr(7);
    }
    
    // 2. Limpiar comillas extras al inicio/final
    while (!content.empty() && (content.front() == '"' || content.front() == '\'' || content.front() == ' ')) {
        content.erase(0, 1);
    }
    while (!content.empty() && (content.back() == '"' || content.back() == '\'' || content.back() == ' ')) {
        content.pop_back();
    }
    
    // 3. Trim espacios
    content.erase(content.begin(), std::find_if(content.begin(), content.end(), 
        [](unsigned char ch) { return !std::isspace(ch); }));
    content.erase(std::find_if(content.rbegin(), content.rend(), 
        [](unsigned char ch) { return !std::isspace(ch); }).base(), content.end());
    
    return content;
}

#ifdef _WIN32
/**
 * 🌐 WINDOWS HTTP CLIENT para Ollama Local
 * Implementación nativa para Windows sin dependencias externas
 */
std::optional<std::string> CallOllamaWindows(
    const std::string& jsonBody,
    const AIConfig& config
) {
    // Parse URL (localhost:11434)
    HINTERNET hSession = WinHttpOpen(
        L"DevilutionX-AI/2.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );
    
    if (!hSession) {
#ifdef _DEBUG
        LogVerbose("AI: WinHttpOpen failed");
#endif
        return std::nullopt;
    }
    
    // Set timeout
    DWORD timeout = config.timeoutMs;
    WinHttpSetOption(hSession, WINHTTP_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    WinHttpSetOption(hSession, WINHTTP_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
    WinHttpSetOption(hSession, WINHTTP_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
    
    // Connect to localhost:11434
    HINTERNET hConnect = WinHttpConnect(
        hSession,
        L"localhost",
        11434,
        0
    );
    
    if (!hConnect) {
#ifdef _DEBUG
        LogVerbose("AI: WinHttpConnect failed - Is Ollama running?");
#endif
        WinHttpCloseHandle(hSession);
        return std::nullopt;
    }
    
    // Open request to /api/generate
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"POST",
        L"/api/generate",
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        0  // HTTP (not HTTPS for localhost)
    );
    
    if (!hRequest) {
#ifdef _DEBUG
        LogVerbose("AI: WinHttpOpenRequest failed");
#endif
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return std::nullopt;
    }
    
    // Set headers
    std::wstring contentType = L"Content-Type: application/json";
    WinHttpAddRequestHeaders(hRequest, contentType.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);
    
    // Send request
    BOOL bResults = WinHttpSendRequest(
        hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        (LPVOID)jsonBody.c_str(),
        jsonBody.length(),
        jsonBody.length(),
        0
    );
    
    if (!bResults) {
#ifdef _DEBUG
        LogVerbose("AI: WinHttpSendRequest failed");
#endif
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return std::nullopt;
    }
    
    // Receive response
    bResults = WinHttpReceiveResponse(hRequest, NULL);
    if (!bResults) {
#ifdef _DEBUG
        LogVerbose("AI: WinHttpReceiveResponse failed");
#endif
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return std::nullopt;
    }
    
    // Read data
    std::string response;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    
    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            break;
        }
        
        if (dwSize == 0) {
            break;
        }
        
        char* buffer = new char[dwSize + 1];
        ZeroMemory(buffer, dwSize + 1);
        
        if (WinHttpReadData(hRequest, buffer, dwSize, &dwDownloaded)) {
            response.append(buffer, dwDownloaded);
        }
        
        delete[] buffer;
    } while (dwSize > 0);
    
    // Cleanup
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    
    if (response.empty()) {
#ifdef _DEBUG
        LogVerbose("AI: Empty response from Ollama");
#endif
        return std::nullopt;
    }
    
    return response;
}
#else
/**
 * 🌐 LINUX/MAC HTTP CLIENT para Ollama Local
 * Implementación con libcurl para plataformas Unix
 */
#ifdef HAVE_CURL
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::optional<std::string> CallOllamaCurl(
    const std::string& jsonBody,
    const AIConfig& config
) {
    CURL* curl = curl_easy_init();
    if (!curl) {
#ifdef _DEBUG
        LogVerbose("AI: curl_easy_init failed");
#endif
        return std::nullopt;
    }
    
    std::string response;
    struct curl_slist* headers = NULL;
    
    // Set headers
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    // Set options for Ollama local
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/generate");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, config.timeoutMs);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
#ifdef _DEBUG
        LogVerbose("AI: curl_easy_perform failed: {} - Is Ollama running?", curl_easy_strerror(res));
#endif
        return std::nullopt;
    }
    
    if (response.empty()) {
#ifdef _DEBUG
        LogVerbose("AI: Empty response from Ollama");
#endif
        return std::nullopt;
    }
    
    return response;
}
#else
// libcurl no disponible - sistema deshabilitado
std::optional<std::string> CallOllamaCurl(
    const std::string& jsonBody,
    const AIConfig& config
) {
#ifdef _DEBUG
    LogVerbose("AI: libcurl not available, AI system disabled");
#endif
    return std::nullopt;
}
#endif // HAVE_CURL
#endif

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
 * 🪙 TOKEN BUCKET: Verifica y resetea tokens diarios
 */
void CheckAndResetDailyTokens() {
    // Obtener día actual (timestamp / 86400 = días desde epoch)
    uint32_t currentDay = SDL_GetTicks() / 86400000;
    
    if (currentDay != g_lastTokenResetDay) {
        g_tokensRemaining = g_aiConfig.tokensPerDay;
        g_lastTokenResetDay = currentDay;
        g_sessionDisabled = false;  // Reset session flag
        
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: Daily tokens reset to {}", g_tokensRemaining);
        }
#endif
    }
}

/**
 * 🛡️ BUDGET CONTROLLED: Verifica si podemos llamar a la IA
 * Ahora con token bucket mejorado
 */
bool CanCallAI() {
    // Check 1: Session disabled?
    if (g_sessionDisabled) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: Session disabled until restart");
        }
#endif
        return false;
    }
    
    // Check 2: Tokens disponibles?
    CheckAndResetDailyTokens();
    
    if (g_tokensRemaining < g_aiConfig.costPerCall) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: No tokens remaining (0/{})", g_aiConfig.tokensPerDay);
        }
#endif
        g_aiStats.tokenBucketRejections++;
        g_sessionDisabled = true;  // 🚫 Hard-disable hasta reinicio
        return false;
    }
    
    // Check 3: Cooldown corto
    uint32_t currentTime = SDL_GetTicks();
    uint32_t minInterval = g_aiConfig.minSecondsBetweenCalls * 1000;
    
    if (currentTime - g_lastAICallTime < minInterval) {
#ifdef _DEBUG
        if (g_debugLogging) {
            uint32_t remaining = (minInterval - (currentTime - g_lastAICallTime)) / 1000;
            LogVerbose("AI: Cooldown active, wait {}s", remaining);
        }
#endif
        return false;
    }
    
    return true;
}

/**
 * Construye el prompt para la IA (Ollama optimizado para qwen2.5)
 */
std::string BuildPrompt(const std::string& text, AITone tone) {
    // Prompt ULTRACORTO - Evita bloqueo cognitivo en CPU
    return StrCat(
        "Rewrite the sentence with a very small change. "
        "Keep the same meaning and tone. Do not add new ideas.\n\n"
        "Sentence: \"", text, "\""
    );
}

/**
 * 🌐 LLAMADA HTTP REAL A OLLAMA LOCAL
 * Implementación multi-plataforma con timeout
 */
std::optional<std::string> CallOllama(
    const std::string& prompt,
    const AIConfig& config
) {
#ifdef _DEBUG
    if (g_debugLogging) {
        LogVerbose("AI: Calling Ollama at localhost:11434...");
    }
#endif
    
    // Construir JSON request para Ollama
    std::string jsonBody = BuildJSONRequest(
        config.model,
        prompt,
        config.temperature,
        config.maxTokens
    );
    
#ifdef _DEBUG
    if (g_debugLogging) {
        LogVerbose("AI: Request body length: {}", jsonBody.length());
    }
#endif
    
    // Llamar Ollama según plataforma
    std::optional<std::string> response;
    
#ifdef _WIN32
    response = CallOllamaWindows(jsonBody, config);
#else
    response = CallOllamaCurl(jsonBody, config);
#endif
    
    if (!response.has_value()) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: HTTP request failed - Is Ollama running?");
        }
#endif
        return std::nullopt;
    }
    
#ifdef _DEBUG
    if (g_debugLogging) {
        LogVerbose("AI: Response length: {}", response->length());
    }
#endif
    
    // Parsear respuesta JSON de Ollama
    auto content = ParseJSONResponse(*response);
    
    if (!content.has_value()) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: Failed to parse JSON response");
        }
#endif
        return std::nullopt;
    }
    
    // Limpiar espacios en blanco
    std::string result = *content;
    // Trim leading whitespace
    result.erase(result.begin(), std::find_if(result.begin(), result.end(), 
        [](unsigned char ch) { return !std::isspace(ch); }));
    // Trim trailing whitespace
    result.erase(std::find_if(result.rbegin(), result.rend(), 
        [](unsigned char ch) { return !std::isspace(ch); }).base(), result.end());
    
#ifdef _DEBUG
    if (g_debugLogging) {
        LogVerbose("AI: Extracted content: {}", result);
    }
#endif
    
    return result;
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
    
    // 🏠 OLLAMA LOCAL: Detectar si está disponible
    // Intentar conectar a localhost:11434/api/tags
    
#ifdef _DEBUG
    LogVerbose("AI: Checking for Ollama at localhost:11434...");
#endif
    
    // TODO: Implementar detección de Ollama con GET /api/tags
    // Por ahora, asumimos que está disponible si el usuario lo configuró
    
    // Auto-habilitar si Ollama está corriendo
    g_aiConfig.enabled = true;  // Se deshabilitará si falla la primera llamada
    
#ifdef _DEBUG
    LogVerbose("AI Text Variation System initialized (Ollama local)");
    LogVerbose("AI: Model: {}", g_aiConfig.model);
    LogVerbose("AI: Endpoint: {}", g_aiConfig.ollamaURL);
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
    if (!g_aiConfig.enabled) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: System disabled");
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
        
        // 🔍 SILENT MODE: Procesar pero no mostrar
        if (g_aiConfig.silentMode) {
            LogAIEvent("cache", "silent_mode", true);
            return std::nullopt;  // Retornar original en modo silencioso
        }
        
        return cacheIt->second;
    }
    
    // 🛡️ Check 4: RATE LIMITING - Token bucket + cooldown
    if (!CanCallAI()) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: Rate limited or no tokens, using original text");
        }
#endif
        g_aiStats.failedRequests++;
        return std::nullopt;  // Fallback a texto original
    }
    
    // 🪙 Consumir token
    g_tokensRemaining -= g_aiConfig.costPerCall;
    g_aiStats.tokensRemaining = g_tokensRemaining;
    
    // Llamar IA
    auto startTime = std::chrono::high_resolution_clock::now();
    
    std::string prompt = BuildPrompt(text, tone);
    auto aiResult = CallOllama(prompt, g_aiConfig);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // 🛡️ REGISTRAR TIEMPO DE LLAMADA (para cooldown)
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
        LogAIEvent("api_call", ToneToString(tone), false);
        return std::nullopt;
    }
    
    // Validar lore safety (ahora con longitud)
    if (!IsLoreSafe(*aiResult, text)) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: Lore/length safety check failed for: {}", *aiResult);
        }
#endif
        g_aiStats.failedRequests++;
        LogAIEvent("validation", ToneToString(tone), false);
        return std::nullopt;
    }
    
    // Éxito: guardar en cache
    g_aiStats.successfulRequests++;
    
    if (g_aiCache.size() >= MAX_CACHE_SIZE) {
        g_aiCache.clear(); // Simple cache eviction
    }
    g_aiCache[cacheKey] = *aiResult;
    
    LogAIEvent("success", ToneToString(tone), true);
    
#ifdef _DEBUG
    if (g_debugLogging) {
        LogVerbose("AI: Success - {} -> {}", text, *aiResult);
        LogVerbose("AI: Tokens remaining: {}/{}", g_tokensRemaining, g_aiConfig.tokensPerDay);
    }
#endif
    
    // 🔍 SILENT MODE: Procesar pero no mostrar
    if (g_aiConfig.silentMode) {
        return std::nullopt;  // Retornar original en modo silencioso
    }
    
    return *aiResult;
}

bool IsLoreSafe(const std::string& aiText, const std::string& baseText)
{
    // 📏 CHECK 1: Longitud relativa (máximo 30% más largo - más permisivo)
    size_t maxLength = static_cast<size_t>(baseText.length() * 1.3f);
    if (aiText.length() > maxLength) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: Length check failed - {} > {} (max)", aiText.length(), maxLength);
        }
#endif
        g_aiStats.lengthRejections++;
        return false;
    }
    
    // 🚫 CHECK 2: Palabras prohibidas (pronombres modernos, asistente, términos meta)
    // PERO: Permitir pronombres si están en el texto original
    static const std::unordered_set<std::string> bannedWords = {
        "i", "you", "your", "my", "assistant", "ai", "help", "sorry", 
        "cannot", "can't", "rewrite", "rewritten", "sentence", "text", 
        "words", "language", "output", "rules", "label"
    };
    
    // Extraer palabras del texto base (para permitir pronombres originales)
    std::unordered_set<std::string> baseWordsLower;
    std::istringstream baseStream(ToLower(baseText));
    std::string baseWord;
    while (baseStream >> baseWord) {
        baseWord.erase(std::remove_if(baseWord.begin(), baseWord.end(),
            [](unsigned char c) { return !std::isalnum(c); }), baseWord.end());
        if (!baseWord.empty()) {
            baseWordsLower.insert(baseWord);
        }
    }
    
    std::istringstream aiStream(ToLower(aiText));
    std::string word;
    while (aiStream >> word) {
        // Limpiar puntuación
        word.erase(std::remove_if(word.begin(), word.end(),
            [](unsigned char c) { return !std::isalnum(c); }), word.end());
        
        if (!word.empty() && bannedWords.find(word) != bannedWords.end()) {
            // ✅ PERMITIR si la palabra está en el texto original
            if (baseWordsLower.find(word) != baseWordsLower.end()) {
                continue;  // Palabra prohibida pero está en el original → OK
            }
            
#ifdef _DEBUG
            if (g_debugLogging) {
                LogVerbose("AI: Banned word detected (not in original): {}", word);
            }
#endif
            g_aiStats.loreSafeRejections++;
            return false;
        }
    }
    
    // ✅ CHECK 3: Validación PERMISIVA
    // Permitir: eliminación, repetición, reordenamiento, sinónimos existentes
    // Bloquear solo: palabras completamente nuevas (no stopwords)
    
    // Palabras comunes permitidas (stopwords + conectores)
    static const std::unordered_set<std::string> commonWords = {
        "a", "an", "the", "is", "are", "was", "were", "be", "been", "being",
        "have", "has", "had", "do", "does", "did", "will", "would", "could",
        "should", "may", "might", "must", "can", "of", "to", "in", "for",
        "on", "at", "by", "with", "from", "as", "into", "through", "during",
        "before", "after", "above", "below", "between", "under", "again",
        "further", "then", "once", "here", "there", "when", "where", "why",
        "how", "all", "both", "each", "few", "more", "most", "other", "some",
        "such", "no", "nor", "not", "only", "own", "same", "so", "than",
        "too", "very", "s", "t", "d", "ll", "ve", "re", "m",
        "hmm", "ah", "oh", "eh", "uh", "huh",
        "aye", "nay", "thee", "thy", "thou", "hath", "doth"
    };
    
    // Extraer palabras del texto base
    std::unordered_set<std::string> baseWords;
    std::istringstream baseStream(ToLower(baseText));
    
    while (baseStream >> word) {
        std::string cleanWord;
        for (char c : word) {
            if (std::isalnum(c)) {
                cleanWord += c;
            }
        }
        if (!cleanWord.empty()) {
            baseWords.insert(cleanWord);
        }
    }
    
    // 📝 CHECK 4: Contar palabras completamente nuevas (MUY PERMISIVO)
    aiStream = std::istringstream(ToLower(aiText));
    int newWordCount = 0;
    int totalWords = 0;
    
    while (aiStream >> word) {
        std::string cleanWord;
        for (char c : word) {
            if (std::isalnum(c)) {
                cleanWord += c;
            }
        }
        
        if (cleanWord.empty()) {
            continue;
        }
        
        totalWords++;
        
        // Solo contar como "nueva" si NO está en base Y NO es stopword
        if (baseWords.find(cleanWord) == baseWords.end() &&
            commonWords.find(cleanWord) == commonWords.end()) {
            newWordCount++;
        }
    }
    
    // Permitir hasta 40% de palabras nuevas (MUY permisivo)
    // Esto permite sinónimos y recortes agresivos
    if (totalWords > 0 && (float)newWordCount / totalWords > 0.4f) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: Too many new words: {}/{} ({}%)", 
                newWordCount, totalWords, (newWordCount * 100 / totalWords));
        }
#endif
        g_aiStats.loreSafeRejections++;
        return false;
    }
    
    return true;
}

std::string ProcessChatMessageWithAI(const std::string& input)
{
    // 🔍 PASO 1: Verificar si hay variantes dormidas (arqueología digital)
    // TODO: Integrar con HiddenContentSystem cuando esté disponible
    // auto dormantVariant = GetDormantTextVariant(input);
    // if (dormantVariant.has_value()) {
    //     return *dormantVariant;  // Prioridad a contenido original
    // }
    
    // 🤖 PASO 2: Si no hay variantes dormidas, intentar IA (micro-variación)
    auto aiResult = TryAITextVariation(input, AITone::Neutral);
    
    if (aiResult.has_value()) {
        return *aiResult;
    }
    
    // 🔙 PASO 3: Fallback final - texto original
    return input;
}

std::string ProcessNPCDialogue(
    const std::string& npcName,
    const std::string& baseText,
    AITone tone
)
{
    // 🏛️ ARQUEOLOGÍA DIGITAL ASISTIDA
    // Pipeline: Dormidos (70%) → IA (25%) → Original (5%)
    
    // 🔍 PASO 1: Buscar variantes dormidas (PRIORIDAD MÁXIMA)
    // Estas son líneas reales del juego original no usadas
    if (g_hiddenContent.IsInitialized()) {
        auto dormantLines = g_hiddenContent.GetDormantLinesForNPC(npcName);
        if (!dormantLines.empty()) {
            // Selección local, sin IA, instantáneo
            size_t index = static_cast<size_t>(GenerateRnd(static_cast<int>(dormantLines.size())));
            
#ifdef _DEBUG
            if (g_debugLogging) {
                LogVerbose("AI: NPC {} - Using dormant line ({} available)", 
                    npcName, dormantLines.size());
            }
#endif
            g_aiStats.totalRequests++;
            g_aiStats.cachedResponses++;  // Contar como cache hit (contenido pre-existente)
            
            return dormantLines[index];  // 70% del contenido
        }
    }
    
    // 🤖 PASO 2: Si no hay dormidas, IA genera micro-variación (30%)
    // La IA solo "desempolva" cuando no hay alternativa
    auto aiResult = TryAITextVariation(baseText, tone);
    
    if (aiResult.has_value()) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: NPC {} - Generated AI variation", npcName);
        }
#endif
        return *aiResult;  // 25% del contenido
    }
    
    // 🔙 PASO 3: Fallback - texto original (siempre funciona)
#ifdef _DEBUG
    if (g_debugLogging) {
        LogVerbose("AI: NPC {} - Using original text (fallback)", npcName);
    }
#endif
    return baseText;  // 5% del contenido
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
    return g_aiConfig.enabled;
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
// 🪙 TOKEN BUCKET SYSTEM
// ============================================================================

int GetRemainingTokens()
{
    CheckAndResetDailyTokens();
    return g_tokensRemaining;
}

void ResetDailyTokens()
{
    g_tokensRemaining = g_aiConfig.tokensPerDay;
    g_lastTokenResetDay = SDL_GetTicks() / 86400000;
    g_sessionDisabled = false;
    
#ifdef _DEBUG
    LogVerbose("AI: Daily tokens manually reset to {}", g_tokensRemaining);
#endif
}

bool HasTokensAvailable()
{
    CheckAndResetDailyTokens();
    return g_tokensRemaining >= g_aiConfig.costPerCall && !g_sessionDisabled;
}

// ============================================================================
// 📊 TELEMETRY SYSTEM
// ============================================================================

void LogAIEvent(const std::string& npc, const std::string& context, bool success)
{
    constexpr size_t MAX_EVENTS = 10;
    
    AIStats::AIEvent event;
    event.npc = npc;
    event.context = context;
    event.timestamp = SDL_GetTicks();
    event.success = success;
    
    g_aiStats.recentEvents.push_back(event);
    
    // Mantener solo los últimos 10 eventos
    if (g_aiStats.recentEvents.size() > MAX_EVENTS) {
        g_aiStats.recentEvents.erase(g_aiStats.recentEvents.begin());
    }
    
#ifdef _DEBUG
    if (g_debugLogging) {
        LogVerbose("AI Event: {} | {} | {}", npc, context, success ? "SUCCESS" : "FAIL");
    }
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
    std::cout << "Silent mode: " << (g_aiConfig.silentMode ? "YES" : "NO") << std::endl;
    std::cout << "Session disabled: " << (g_sessionDisabled ? "YES" : "NO") << std::endl;
    
    std::cout << "\n--- TOKEN BUCKET ---" << std::endl;
    std::cout << "Tokens remaining: " << g_tokensRemaining << "/" << g_aiConfig.tokensPerDay << std::endl;
    std::cout << "Cost per call: " << g_aiConfig.costPerCall << std::endl;
    std::cout << "Cooldown: " << g_aiConfig.minSecondsBetweenCalls << "s" << std::endl;
    std::cout << "Token rejections: " << g_aiStats.tokenBucketRejections << std::endl;
    
    std::cout << "\n--- REQUESTS ---" << std::endl;
    std::cout << "Total requests: " << g_aiStats.totalRequests << std::endl;
    std::cout << "Successful: " << g_aiStats.successfulRequests << std::endl;
    std::cout << "Failed: " << g_aiStats.failedRequests << std::endl;
    std::cout << "Cached responses: " << g_aiStats.cachedResponses << std::endl;
    
    std::cout << "\n--- VALIDATION ---" << std::endl;
    std::cout << "Lore-safe rejections: " << g_aiStats.loreSafeRejections << std::endl;
    std::cout << "Length rejections: " << g_aiStats.lengthRejections << std::endl;
    
    std::cout << "\n--- PERFORMANCE ---" << std::endl;
    std::cout << "Average latency: " << g_aiStats.averageLatencyMs << "ms" << std::endl;
    std::cout << "Cache size: " << g_aiCache.size() << "/" << MAX_CACHE_SIZE << std::endl;
    
    std::cout << "\n--- RECENT EVENTS (Last 10) ---" << std::endl;
    for (const auto& event : g_aiStats.recentEvents) {
        std::cout << "[" << event.timestamp << "] " 
                  << event.npc << " | " << event.context << " | "
                  << (event.success ? "✓" : "✗") << std::endl;
    }
    
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
