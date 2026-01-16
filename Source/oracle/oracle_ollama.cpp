/**
 * @file oracle_ollama.cpp
 * 
 * 🔮 EL ORÁCULO - Implementación del Cliente Ollama
 */

#include "oracle_ollama.h"

#include <thread>
#include <sstream>

#include "utils/log.hpp"
#include "utils/str_cat.hpp"

// Platform-specific HTTP client
#ifdef _WIN32
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#else
// TODO: Add libcurl support for Linux/Mac if needed
#endif

namespace devilution {

// ============================================================================
// 🔮 CONFIGURACIÓN
// ============================================================================

constexpr const char* OLLAMA_HOST = "localhost";
constexpr int OLLAMA_PORT = 11434;
constexpr const char* OLLAMA_PATH = "/api/generate";
constexpr const char* OLLAMA_MODEL = "qwen2.5:3b-instruct";
constexpr int OLLAMA_TIMEOUT_MS = 5000;  // 5 segundos (más rápido que IA)
constexpr int PING_TIMEOUT_MS = 1000;    // 1 segundo para ping

// ============================================================================
// 🔮 IMPLEMENTACIÓN
// ============================================================================

bool OracleOllama::IsAvailable()
{
#ifdef _WIN32
	// Ping rápido a Ollama
	HINTERNET hSession = WinHttpOpen(
		L"NightmareEdition/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0
	);
	
	if (!hSession) {
		return false;
	}
	
	HINTERNET hConnect = WinHttpConnect(
		hSession,
		L"localhost",
		OLLAMA_PORT,
		0
	);
	
	if (!hConnect) {
		WinHttpCloseHandle(hSession);
		return false;
	}
	
	HINTERNET hRequest = WinHttpOpenRequest(
		hConnect,
		L"GET",
		L"/",
		nullptr,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		0
	);
	
	if (!hRequest) {
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return false;
	}
	
	// Timeout corto
	WinHttpSetTimeouts(hRequest, PING_TIMEOUT_MS, PING_TIMEOUT_MS, PING_TIMEOUT_MS, PING_TIMEOUT_MS);
	
	BOOL result = WinHttpSendRequest(
		hRequest,
		WINHTTP_NO_ADDITIONAL_HEADERS,
		0,
		WINHTTP_NO_REQUEST_DATA,
		0,
		0,
		0
	);
	
	if (result) {
		result = WinHttpReceiveResponse(hRequest, nullptr);
	}
	
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
	
	return result == TRUE;
#else
	// TODO: Implement for Linux/Mac with libcurl
	LogVerbose("Oracle: Ollama client not implemented for this platform");
	return false;
#endif
}

void OracleOllama::QueryAsync(
	const std::string& prompt,
	std::function<void(std::optional<std::string>)> callback
)
{
	// Lanzar thread para no bloquear el juego
	std::thread([prompt, callback]() {
		auto response = QuerySync(prompt);
		callback(response);
	}).detach();
}

std::optional<std::string> OracleOllama::QuerySync(const std::string& prompt)
{
#ifdef _WIN32
	std::string url = StrCat("http://", OLLAMA_HOST, ":", OLLAMA_PORT, OLLAMA_PATH);
	std::string jsonBody = BuildJSONRequest(prompt);
	
#ifdef _DEBUG
	LogVerbose("Oracle: Querying Ollama with prompt: \"{}\"", prompt);
#endif
	
	auto response = HttpPost(url, jsonBody, OLLAMA_TIMEOUT_MS);
	
	if (!response.has_value()) {
#ifdef _DEBUG
		LogVerbose("Oracle: Ollama query failed (no response)");
#endif
		return std::nullopt;
	}
	
	auto parsed = ParseJSONResponse(*response);
	
	if (!parsed.has_value()) {
#ifdef _DEBUG
		LogVerbose("Oracle: Failed to parse Ollama response");
#endif
		return std::nullopt;
	}
	
#ifdef _DEBUG
	LogVerbose("Oracle: Ollama response: \"{}\"", *parsed);
#endif
	
	return parsed;
#else
	// TODO: Implement for Linux/Mac
	LogVerbose("Oracle: Ollama client not implemented for this platform");
	return std::nullopt;
#endif
}

std::string OracleOllama::BuildJSONRequest(const std::string& prompt)
{
	// Escapar comillas en el prompt
	std::string escapedPrompt;
	escapedPrompt.reserve(prompt.size() + 10);
	
	for (char c : prompt) {
		if (c == '"' || c == '\\') {
			escapedPrompt += '\\';
		}
		escapedPrompt += c;
	}
	
	// Construir JSON manualmente (simple y rápido)
	return StrCat(
		"{",
		"\"model\":\"", OLLAMA_MODEL, "\",",
		"\"prompt\":\"", escapedPrompt, "\",",
		"\"stream\":false,",
		"\"options\":{",
			"\"temperature\":0.8,",
			"\"top_p\":0.9,",
			"\"top_k\":40,",
			"\"num_predict\":150",
		"}",
		"}"
	);
}

std::optional<std::string> OracleOllama::ParseJSONResponse(const std::string& json)
{
	// Parser JSON simple (buscar "response":"...")
	// No usamos librería JSON para mantener dependencias mínimas
	
	size_t responsePos = json.find("\"response\"");
	if (responsePos == std::string::npos) {
		return std::nullopt;
	}
	
	size_t colonPos = json.find(':', responsePos);
	if (colonPos == std::string::npos) {
		return std::nullopt;
	}
	
	size_t quoteStart = json.find('"', colonPos);
	if (quoteStart == std::string::npos) {
		return std::nullopt;
	}
	
	size_t quoteEnd = quoteStart + 1;
	while (quoteEnd < json.size()) {
		if (json[quoteEnd] == '"' && json[quoteEnd - 1] != '\\') {
			break;
		}
		quoteEnd++;
	}
	
	if (quoteEnd >= json.size()) {
		return std::nullopt;
	}
	
	std::string response = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
	
	// Desescapar \n
	size_t pos = 0;
	while ((pos = response.find("\\n", pos)) != std::string::npos) {
		response.replace(pos, 2, "\n");
		pos += 1;
	}
	
	// Trim whitespace
	size_t start = response.find_first_not_of(" \t\n\r");
	size_t end = response.find_last_not_of(" \t\n\r");
	
	if (start == std::string::npos || end == std::string::npos) {
		return std::nullopt;
	}
	
	return response.substr(start, end - start + 1);
}

std::optional<std::string> OracleOllama::HttpPost(
	const std::string& url,
	const std::string& jsonBody,
	int timeoutMs
)
{
#ifdef _WIN32
	// Convertir URL a wide string
	std::wstring wHost(OLLAMA_HOST, OLLAMA_HOST + strlen(OLLAMA_HOST));
	std::wstring wPath(OLLAMA_PATH, OLLAMA_PATH + strlen(OLLAMA_PATH));
	
	// Abrir sesión
	HINTERNET hSession = WinHttpOpen(
		L"NightmareEdition/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS,
		0
	);
	
	if (!hSession) {
		return std::nullopt;
	}
	
	// Conectar
	HINTERNET hConnect = WinHttpConnect(
		hSession,
		wHost.c_str(),
		OLLAMA_PORT,
		0
	);
	
	if (!hConnect) {
		WinHttpCloseHandle(hSession);
		return std::nullopt;
	}
	
	// Abrir request
	HINTERNET hRequest = WinHttpOpenRequest(
		hConnect,
		L"POST",
		wPath.c_str(),
		nullptr,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		0
	);
	
	if (!hRequest) {
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return std::nullopt;
	}
	
	// Configurar timeouts
	WinHttpSetTimeouts(hRequest, timeoutMs, timeoutMs, timeoutMs, timeoutMs);
	
	// Headers
	std::wstring headers = L"Content-Type: application/json\r\n";
	
	// Enviar request
	BOOL result = WinHttpSendRequest(
		hRequest,
		headers.c_str(),
		-1,
		(LPVOID)jsonBody.c_str(),
		jsonBody.size(),
		jsonBody.size(),
		0
	);
	
	if (!result) {
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return std::nullopt;
	}
	
	// Recibir respuesta
	result = WinHttpReceiveResponse(hRequest, nullptr);
	
	if (!result) {
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return std::nullopt;
	}
	
	// Leer datos
	std::string responseData;
	DWORD bytesAvailable = 0;
	DWORD bytesRead = 0;
	char buffer[4096];
	
	do {
		bytesAvailable = 0;
		if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable)) {
			break;
		}
		
		if (bytesAvailable == 0) {
			break;
		}
		
		DWORD bytesToRead = (bytesAvailable > sizeof(buffer)) ? sizeof(buffer) : bytesAvailable;
		
		if (!WinHttpReadData(hRequest, buffer, bytesToRead, &bytesRead)) {
			break;
		}
		
		responseData.append(buffer, bytesRead);
		
	} while (bytesAvailable > 0);
	
	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);
	
	if (responseData.empty()) {
		return std::nullopt;
	}
	
	return responseData;
#else
	// TODO: Implement for Linux/Mac with libcurl
	return std::nullopt;
#endif
}

} // namespace devilution
