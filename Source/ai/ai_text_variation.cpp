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
 * 🌐 SIMPLE JSON BUILDER (sin dependencias externas)
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
    
    return StrCat(
        "{\"model\":\"", model, "\","
        "\"messages\":[{\"role\":\"user\",\"content\":\"", escapedPrompt, "\"}],"
        "\"temperature\":", std::to_string(temperature), ","
        "\"max_tokens\":", std::to_string(maxTokens), "}"
    );
}

/**
 * 🌐 SIMPLE JSON PARSER (sin dependencias externas)
 * Extrae el contenido de la respuesta JSON
 */
std::optional<std::string> ParseJSONResponse(const std::string& json) {
    // Buscar: "content":"..."
    size_t contentPos = json.find("\"content\"");
    if (contentPos == std::string::npos) {
        return std::nullopt;
    }
    
    // Buscar el inicio del valor (después de ":")
    size_t valueStart = json.find(':', contentPos);
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
    
    return content;
}

#ifdef _WIN32
/**
 * 🌐 WINDOWS HTTP CLIENT (WinHTTP)
 * Implementación nativa para Windows sin dependencias externas
 */
std::optional<std::string> CallOpenRouterWindows(
    const std::string& jsonBody,
    const AIConfig& config
) {
    // Parse URL
    HINTERNET hSession = WinHttpOpen(
        L"DevilutionX-AI/1.0",
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
    
    // Connect
    HINTERNET hConnect = WinHttpConnect(
        hSession,
        L"openrouter.ai",
        INTERNET_DEFAULT_HTTPS_PORT,
        0
    );
    
    if (!hConnect) {
#ifdef _DEBUG
        LogVerbose("AI: WinHttpConnect failed");
#endif
        WinHttpCloseHandle(hSession);
        return std::nullopt;
    }
    
    // Open request
    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"POST",
        L"/api/v1/chat/completions",
        NULL,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE
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
    std::wstring authHeader = L"Authorization: Bearer " + 
        std::wstring(config.apiKey.begin(), config.apiKey.end());
    std::wstring contentType = L"Content-Type: application/json";
    
    WinHttpAddRequestHeaders(hRequest, authHeader.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD);
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
        LogVerbose("AI: Empty response");
#endif
        return std::nullopt;
    }
    
    return response;
}
#else
/**
 * 🌐 LINUX/MAC HTTP CLIENT (libcurl)
 * Implementación con libcurl para plataformas Unix
 */
#ifdef HAVE_CURL
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::optional<std::string> CallOpenRouterCurl(
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
    std::string authHeader = "Authorization: Bearer " + config.apiKey;
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    // Set options
    curl_easy_setopt(curl, CURLOPT_URL, "https://openrouter.ai/api/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, config.timeoutMs);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    // Cleanup
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
#ifdef _DEBUG
        LogVerbose("AI: curl_easy_perform failed: {}", curl_easy_strerror(res));
#endif
        return std::nullopt;
    }
    
    if (response.empty()) {
#ifdef _DEBUG
        LogVerbose("AI: Empty response");
#endif
        return std::nullopt;
    }
    
    return response;
}
#else
// libcurl no disponible - sistema deshabilitado
std::optional<std::string> CallOpenRouterCurl(
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
 * 🌐 LLAMADA HTTP REAL A OPENROUTER
 * Implementación multi-plataforma con timeout agresivo
 */
std::optional<std::string> CallOpenRouter(
    const std::string& prompt,
    const AIConfig& config
) {
#ifdef _DEBUG
    if (g_debugLogging) {
        LogVerbose("AI: Calling OpenRouter API...");
    }
#endif
    
    // Construir JSON request
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
    
    // Llamar API según plataforma
    std::optional<std::string> response;
    
#ifdef _WIN32
    response = CallOpenRouterWindows(jsonBody, config);
#else
    response = CallOpenRouterCurl(jsonBody, config);
#endif
    
    if (!response.has_value()) {
#ifdef _DEBUG
        if (g_debugLogging) {
            LogVerbose("AI: HTTP request failed");
        }
#endif
        return std::nullopt;
    }
    
#ifdef _DEBUG
    if (g_debugLogging) {
        LogVerbose("AI: Response length: {}", response->length());
    }
#endif
    
    // Parsear respuesta JSON
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
    
    // 🔧 LEER API KEY AUTOMÁTICAMENTE desde tools/.env.dev
    // Esto permite que funcione sin configuración manual
    std::string envPath = "tools/.env.dev";
    std::ifstream envFile(envPath);
    if (envFile.is_open()) {
        std::string line;
        while (std::getline(envFile, line)) {
            if (line.find("API_KEY=") == 0) {
                // Extraer API key (formato: API_KEY="sk-or-v1-...")
                std::string key = line.substr(8); // Después de "API_KEY="
                // Remover comillas si existen
                key.erase(std::remove(key.begin(), key.end(), '"'), key.end());
                key.erase(std::remove(key.begin(), key.end(), '\''), key.end());
                
                if (!key.empty()) {
                    g_aiConfig.apiKey = key;
                    g_aiConfig.enabled = true; // ✅ Auto-habilitar si hay API key
                    
#ifdef _DEBUG
                    LogVerbose("AI: API key loaded from tools/.env.dev");
                    LogVerbose("AI: System auto-enabled");
#endif
                }
                break;
            }
        }
        envFile.close();
    }
    
    // Si no hay API key, sistema deshabilitado
    if (g_aiConfig.apiKey.empty()) {
        g_aiConfig.enabled = false;
#ifdef _DEBUG
        LogVerbose("AI: No API key found, system disabled");
#endif
    }
    
#ifdef _DEBUG
    LogVerbose("AI Text Variation System initialized");
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
