/**
 * @file oracle_ollama.h
 * 
 * 🔮 EL ORÁCULO - Cliente Ollama
 * 
 * Cliente HTTP asíncrono para conectar con Ollama local
 * y generar respuestas crípticas del Oráculo.
 * 
 * FILOSOFÍA:
 * - Nunca bloquea el gameplay
 * - Timeout corto (5 segundos)
 * - Funciona 100% normal si Ollama no está disponible
 * - Thread-safe
 */

#pragma once

#include <string>
#include <optional>
#include <functional>

namespace devilution {

/**
 * @brief Cliente Ollama para el Oráculo
 * 
 * Wrapper asíncrono sobre HTTP para consultar Ollama local.
 * Usa threads para no bloquear el juego.
 */
class OracleOllama {
public:
	/**
	 * @brief Verifica si Ollama está disponible
	 * 
	 * Hace un ping rápido a Ollama para verificar conectividad.
	 * 
	 * @return true si Ollama responde en menos de 1 segundo
	 */
	static bool IsAvailable();
	
	/**
	 * @brief Query asíncrono a Ollama
	 * 
	 * Ejecuta la consulta en un thread separado y llama al callback
	 * cuando la respuesta está lista (o timeout).
	 * 
	 * @param prompt Prompt completo para Ollama
	 * @param callback Función a llamar con la respuesta (o nullopt si falla)
	 * 
	 * @note El callback se ejecuta en el thread de Ollama, no en el main thread.
	 *       Usar mutex si se accede a estado compartido.
	 */
	static void QueryAsync(
		const std::string& prompt,
		std::function<void(std::optional<std::string>)> callback
	);
	
private:
	/**
	 * @brief Query síncrono a Ollama (bloqueante)
	 * 
	 * @param prompt Prompt completo
	 * @return Respuesta de Ollama o nullopt si falla
	 */
	static std::optional<std::string> QuerySync(const std::string& prompt);
	
	/**
	 * @brief Construye el JSON request para Ollama
	 * @param prompt Prompt del usuario
	 * @return JSON string
	 */
	static std::string BuildJSONRequest(const std::string& prompt);
	
	/**
	 * @brief Parsea la respuesta JSON de Ollama
	 * @param json Respuesta JSON de Ollama
	 * @return Texto generado o nullopt si falla
	 */
	static std::optional<std::string> ParseJSONResponse(const std::string& json);
	
	/**
	 * @brief Hace un HTTP POST a Ollama
	 * @param url URL completa
	 * @param jsonBody Body JSON
	 * @param timeoutMs Timeout en milisegundos
	 * @return Respuesta HTTP o nullopt si falla
	 */
	static std::optional<std::string> HttpPost(
		const std::string& url,
		const std::string& jsonBody,
		int timeoutMs
	);
};

} // namespace devilution
